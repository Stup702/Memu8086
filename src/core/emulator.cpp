#include "emulator.h"
#include <sstream>

namespace emu8086::core {

// --- Internal IODevice Implementation ---
class StringIODevice : public IODevice {
public:
    StringIODevice(Emulator& emu) : emu_(emu) {}

    void write_char(char c) override {
        std::lock_guard<std::mutex> lock(emu_.output_mutex_);
        emu_.output_buffer_.push_back(c);
    }

    int read_char() override {
        // Fallback interface requirement (InterruptHandler queue is accessed natively)
        return -1;
    }

    void clear_screen() override {
        std::lock_guard<std::mutex> lock(emu_.output_mutex_);
        emu_.output_buffer_.push_back('\f');
    }
private:
    Emulator& emu_;
};

// --- Emulator Orchestrator ---
Emulator::Emulator() {
    io_device_ = std::make_unique<StringIODevice>(*this);
    irq_ = std::make_unique<InterruptHandler>(cpu_, *io_device_);
    executor_ = std::make_unique<Executor>(cpu_);
}

Emulator::~Emulator() {
    stop();
}

bool Emulator::load_com(const std::vector<uint8_t>& machine_code, uint16_t load_offset) {
    stop();
    std::lock_guard<std::mutex> lock(cpu_mutex_);
    cpu_.reset();
    executor_->reset();
    irq_->halted = false;
    irq_->warnings.clear();
    
    for (size_t i = 0; i < machine_code.size(); ++i) {
        if (load_offset + i < MEMORY_SIZE) {
            cpu_.mem.write8(load_offset + i, machine_code[i]);
        }
    }
    
    cpu_.regs.CS = 0x0000;
    cpu_.regs.IP = load_offset;
    cpu_.regs.SS = 0x0000;
    cpu_.regs.SP = 0xFFFE;
    
    state_ = EmulatorState::IDLE;
    return true;
}

bool Emulator::load_from_assembly(const std::string& source) {
    assembler::Assembler asmb;
    last_asm_ = asmb.assemble(source, 0x0100);
    if (last_asm_.success) {
        return load_com(last_asm_.machine_code, 0x0100);
    }
    return false;
}

const assembler::AssemblyResult& Emulator::last_assembly() const {
    return last_asm_;
}

void Emulator::start() {
    if (state_ == EmulatorState::RUNNING) return;
    if (exec_thread_.joinable()) exec_thread_.join();
    state_ = EmulatorState::RUNNING;
    run_target_ip_ = 0xFFFF;
    exec_thread_ = std::thread(&Emulator::exec_loop_, this);
}

void Emulator::pause() {
    if (state_ == EmulatorState::RUNNING) {
        state_ = EmulatorState::PAUSED;
    }
}

void Emulator::stop() {
    state_ = EmulatorState::IDLE;
    if (exec_thread_.joinable()) {
        for (int i = 0; i < 50; ++i) { // Wait up to 500ms
            if (!thread_active_) break;
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        if (thread_active_) exec_thread_.detach();
        else exec_thread_.join();
    }
    std::lock_guard<std::mutex> lock(cpu_mutex_);
    cpu_.reset();
    executor_->reset();
    irq_->halted = false;
    irq_->warnings.clear();
    {
        std::lock_guard<std::mutex> out_lock(output_mutex_);
        output_buffer_.clear();
    }
}

void Emulator::step() {
    if (state_ != EmulatorState::IDLE && state_ != EmulatorState::PAUSED) return;
    std::lock_guard<std::mutex> lock(cpu_mutex_);
    
    try {
        ExecResult res = executor_->step();
        if (res == ExecResult::INTERRUPT_PENDING) {
            irq_->handle(executor_->last_interrupt_num);
        }
        if (irq_->halted || res == ExecResult::HALT) state_ = EmulatorState::HALTED;
        else if (res == ExecResult::ILLEGAL_OPCODE || res == ExecResult::DIVISION_BY_ZERO) state_ = EmulatorState::ERROR;
    } catch (const std::exception& e) {
        state_ = EmulatorState::ERROR;
        std::lock_guard<std::mutex> out_lock(output_mutex_);
        output_buffer_.append(std::string("\n[EMULATOR EXCEPTION] ") + e.what() + "\n");
    } catch (...) {
        state_ = EmulatorState::ERROR;
    }
}

void Emulator::step_over() {
    if (state_ != EmulatorState::IDLE && state_ != EmulatorState::PAUSED) return;
    
    std::unique_lock<std::mutex> lock(cpu_mutex_);
    uint8_t op = cpu_.mem.read8(cpu_.cs_ip());
    
    // Approximate CALL recognition (0xE8 = near, 0x9A = far, 0xFF modrm /2 or /3 = indirect)
    bool is_call = (op == 0xE8 || op == 0x9A || 
                   (op == 0xFF && ((cpu_.mem.read8(cpu_.cs_ip() + 1) >> 3) & 7) == 2) || 
                   (op == 0xFF && ((cpu_.mem.read8(cpu_.cs_ip() + 1) >> 3) & 7) == 3));
    
    if (is_call) {
        uint16_t prev_sp = cpu_.regs.SP;
        ExecResult res = executor_->step();
        
        if (res == ExecResult::INTERRUPT_PENDING) irq_->handle(executor_->last_interrupt_num);
        
        if (cpu_.regs.SP < prev_sp) { // Instruction pushed return offset
            uint16_t ret_addr = cpu_.mem.read16(cpu_.ss_sp());
            lock.unlock();
            run_to(ret_addr);
            return;
        }
    } else {
        ExecResult res = executor_->step();
        if (res == ExecResult::INTERRUPT_PENDING) irq_->handle(executor_->last_interrupt_num);
    }
    
    if (irq_->halted) state_ = EmulatorState::HALTED;
}

void Emulator::run_to(uint16_t ip_target) {
    if (state_ == EmulatorState::RUNNING) return;
    if (exec_thread_.joinable()) exec_thread_.join();
    state_ = EmulatorState::RUNNING;
    run_target_ip_ = ip_target;
    exec_thread_ = std::thread(&Emulator::exec_loop_, this);
}

void Emulator::exec_loop_() {
    auto last_tick = std::chrono::steady_clock::now();
    
    while (state_ == EmulatorState::RUNNING) {
        std::unique_lock<std::mutex> lock(cpu_mutex_);
        
        ExecResult res = executor_->step();
        
        if (res == ExecResult::INTERRUPT_PENDING) {
            irq_->handle(executor_->last_interrupt_num);
        }
        
        // Breakpoint/HALT resolution executes AFTER processing the current step 
        // cleanly avoiding resuming issues on top of a breakpoint.
        if (res == ExecResult::HALT || irq_->halted) { state_ = EmulatorState::HALTED; break; }
        if (res == ExecResult::ILLEGAL_OPCODE || res == ExecResult::DIVISION_BY_ZERO) { state_ = EmulatorState::ERROR; break; }
        if (res == ExecResult::BREAKPOINT || breakpoints_.count(cpu_.regs.IP)) { state_ = EmulatorState::PAUSED; break; }
        if (run_target_ip_ != 0xFFFF && cpu_.regs.IP == run_target_ip_) {
            state_ = EmulatorState::PAUSED;
            run_target_ip_ = 0xFFFF; // single-shot expiration
            break;
        }
        
        lock.unlock();
        if (speed_hz_ > 0) throttle_(last_tick);
    }
}

void Emulator::throttle_(std::chrono::steady_clock::time_point& last_tick) {
    uint32_t hz = speed_hz_.load();
    if (hz == 0) return;
    
    auto expected_ns = std::chrono::nanoseconds(1000000000ULL / hz);
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(now - last_tick);
    
    if (elapsed < expected_ns) {
        auto wait_time = expected_ns - elapsed;
        if (wait_time > std::chrono::microseconds(500)) {
            std::this_thread::sleep_for(wait_time);
        } else {
            // Fine-grained spin for smaller than OS scheduler minimum slices
            while (std::chrono::steady_clock::now() - last_tick < expected_ns) {}
        }
    }
    last_tick = std::chrono::steady_clock::now();
}

void Emulator::add_breakpoint(uint16_t ip) { breakpoints_.insert(ip); }
void Emulator::remove_breakpoint(uint16_t ip) { breakpoints_.erase(ip); }
void Emulator::clear_breakpoints() { breakpoints_.clear(); }
bool Emulator::has_breakpoint(uint16_t ip) const { return breakpoints_.count(ip) > 0; }
const std::unordered_set<uint16_t>& Emulator::breakpoints() const { return breakpoints_; }
void Emulator::toggle_breakpoint(uint16_t ip) {
    if (has_breakpoint(ip)) remove_breakpoint(ip);
    else add_breakpoint(ip);
}

void Emulator::request_memory_view(uint32_t addr) { memory_goto_request_ = addr; }
uint32_t Emulator::consume_memory_view_request() { return memory_goto_request_.exchange(0xFFFFFFFF); }

void Emulator::toggle_flag(int flag_idx) {
    std::lock_guard<std::mutex> lock(cpu_mutex_);
    switch (flag_idx) {
        case 0: cpu_.regs.flags.CF = !cpu_.regs.flags.CF; break;
        case 1: cpu_.regs.flags.PF = !cpu_.regs.flags.PF; break;
        case 2: cpu_.regs.flags.AF = !cpu_.regs.flags.AF; break;
        case 3: cpu_.regs.flags.ZF = !cpu_.regs.flags.ZF; break;
        case 4: cpu_.regs.flags.SF = !cpu_.regs.flags.SF; break;
        case 5: cpu_.regs.flags.TF = !cpu_.regs.flags.TF; break;
        case 6: cpu_.regs.flags.IF = !cpu_.regs.flags.IF; break;
        case 7: cpu_.regs.flags.DF = !cpu_.regs.flags.DF; break;
        case 8: cpu_.regs.flags.OF = !cpu_.regs.flags.OF; break;
    }
}

EmulatorState Emulator::state() const { return state_.load(); }
void Emulator::set_speed_hz(uint32_t hz) { speed_hz_ = hz; }
uint32_t Emulator::get_speed_hz() const { return speed_hz_.load(); }
void Emulator::send_key(char c) { irq_->enqueue_key(c); }
const std::array<uint8_t, MEMORY_SIZE>& Emulator::memory_view() const { return cpu_.mem.get_data(); }

void Emulator::write_memory(uint32_t addr, uint8_t val) {
    std::lock_guard<std::mutex> lock(cpu_mutex_);
    if (addr < MEMORY_SIZE) cpu_.mem.write8(addr, val);
}

EmulatorSnapshot Emulator::snapshot() const {
    std::lock_guard<std::mutex> lock(cpu_mutex_);
    EmulatorSnapshot snap;
    snap.regs = cpu_.regs;
    snap.flags = cpu_.regs.flags;
    snap.cycle_count = executor_->cycle_count;
    if (!irq_->warnings.empty()) snap.last_error = irq_->warnings.back();
    return snap;
}

std::vector<std::string> Emulator::io_output() {
    std::string local_buf;
    {
        std::lock_guard<std::mutex> lock(output_mutex_);
        local_buf = std::move(output_buffer_);
        output_buffer_.clear();
    }
    
    std::vector<std::string> lines;
    if (local_buf.empty()) return lines;
    
    size_t start = 0;
    size_t end = local_buf.find('\n');
    while (end != std::string::npos) {
        lines.push_back(local_buf.substr(start, end - start));
        start = end + 1;
        end = local_buf.find('\n', start);
    }
    if (start < local_buf.length()) lines.push_back(local_buf.substr(start));
    
    return lines;
}

} // namespace emu8086::core