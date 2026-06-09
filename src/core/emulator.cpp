#include "emulator.h"
#include <exception>

namespace emu8086::core {

static uint16_t s_target_cs = 0xFFFF;

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
    irq_ = std::make_unique<InterruptHandler>(cpu_, *io_device_);
    
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
    s_target_cs = 0xFFFF;
    exec_thread_ = std::thread(&Emulator::exec_loop_, this);
}

void Emulator::pause() {
    if (state_ == EmulatorState::RUNNING) {
        state_ = EmulatorState::PAUSED;
    }
}

void Emulator::stop() {
    state_ = EmulatorState::IDLE;
    if (irq_) {
        irq_->enqueue_key('\0');
    }
    if (exec_thread_.joinable()) {
        exec_thread_.join();
    }
    std::lock_guard<std::mutex> lock(cpu_mutex_);
    cpu_.reset();
    executor_->reset();
    irq_ = std::make_unique<InterruptHandler>(cpu_, *io_device_);
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
            if (!irq_->handle(executor_->last_interrupt_num)) {
                uint8_t vec = executor_->last_interrupt_num;
                cpu_.regs.SP -= 2; cpu_.mem.write16(cpu_.ss_sp(), cpu_.regs.flags.to_word());
                cpu_.regs.flags.IF = false; cpu_.regs.flags.TF = false;
                cpu_.regs.SP -= 2; cpu_.mem.write16(cpu_.ss_sp(), cpu_.regs.CS);
                cpu_.regs.SP -= 2; cpu_.mem.write16(cpu_.ss_sp(), cpu_.regs.IP);
                cpu_.regs.IP = cpu_.mem.read16(vec * 4);
                cpu_.regs.CS = cpu_.mem.read16(vec * 4 + 2);
            }
        } else if (res == ExecResult::DIVISION_BY_ZERO) {
            uint8_t vec = 0;
            cpu_.regs.SP -= 2; cpu_.mem.write16(cpu_.ss_sp(), cpu_.regs.flags.to_word());
            cpu_.regs.flags.IF = false; cpu_.regs.flags.TF = false;
            cpu_.regs.SP -= 2; cpu_.mem.write16(cpu_.ss_sp(), cpu_.regs.CS);
            cpu_.regs.SP -= 2; cpu_.mem.write16(cpu_.ss_sp(), cpu_.regs.IP);
            cpu_.regs.IP = cpu_.mem.read16(vec * 4);
            cpu_.regs.CS = cpu_.mem.read16(vec * 4 + 2);
            res = ExecResult::OK;
        }
            
        if (irq_->halted || res == ExecResult::HALT) state_ = EmulatorState::HALTED;
        else if (res == ExecResult::ILLEGAL_OPCODE) state_ = EmulatorState::ERROR;
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
    
    uint32_t peek_addr = cpu_.cs_ip();
    uint8_t op = cpu_.mem.read8(peek_addr);
    while (op == 0x26 || op == 0x2E || op == 0x36 || op == 0x3E || op == 0xF0 || op == 0xF2 || op == 0xF3) {
        peek_addr++;
        op = cpu_.mem.read8(peek_addr);
    }
    
    // Approximate CALL recognition (0xE8 = near, 0x9A = far, 0xFF modrm /2 or /3 = indirect)
    bool is_call = (op == 0xE8 || op == 0x9A || 
                   (op == 0xFF && ((cpu_.mem.read8(peek_addr + 1) >> 3) & 7) == 2) || 
                   (op == 0xFF && ((cpu_.mem.read8(peek_addr + 1) >> 3) & 7) == 3) ||
                   op == 0xCC || op == 0xCD || op == 0xCE);
    
    ExecResult res;
    
    if (is_call) {
        uint16_t prev_sp = cpu_.regs.SP;
        uint16_t expected_ret_cs = cpu_.regs.CS;
        res = executor_->step();
        
        if (res == ExecResult::INTERRUPT_PENDING) {
            if (!irq_->handle(executor_->last_interrupt_num)) {
                uint8_t vec = executor_->last_interrupt_num;
                cpu_.regs.SP -= 2; cpu_.mem.write16(cpu_.ss_sp(), cpu_.regs.flags.to_word());
                cpu_.regs.flags.IF = false; cpu_.regs.flags.TF = false;
                cpu_.regs.SP -= 2; cpu_.mem.write16(cpu_.ss_sp(), cpu_.regs.CS);
                cpu_.regs.SP -= 2; cpu_.mem.write16(cpu_.ss_sp(), cpu_.regs.IP);
                cpu_.regs.IP = cpu_.mem.read16(vec * 4);
                cpu_.regs.CS = cpu_.mem.read16(vec * 4 + 2);
            }
        } else if (res == ExecResult::DIVISION_BY_ZERO) {
            uint8_t vec = 0;
            cpu_.regs.SP -= 2; cpu_.mem.write16(cpu_.ss_sp(), cpu_.regs.flags.to_word());
            cpu_.regs.flags.IF = false; cpu_.regs.flags.TF = false;
            cpu_.regs.SP -= 2; cpu_.mem.write16(cpu_.ss_sp(), cpu_.regs.CS);
            cpu_.regs.SP -= 2; cpu_.mem.write16(cpu_.ss_sp(), cpu_.regs.IP);
            cpu_.regs.IP = cpu_.mem.read16(vec * 4);
            cpu_.regs.CS = cpu_.mem.read16(vec * 4 + 2);
            res = ExecResult::OK;
        }
        
        if (cpu_.regs.SP < prev_sp) { // Instruction pushed return offset
            uint16_t ret_addr = cpu_.mem.read16(cpu_.ss_sp());
            s_target_cs = expected_ret_cs;
            lock.unlock();
            run_to(ret_addr);
            return;
        }
    } else {
        res = executor_->step();
        if (res == ExecResult::INTERRUPT_PENDING) {
            if (!irq_->handle(executor_->last_interrupt_num)) {
                uint8_t vec = executor_->last_interrupt_num;
                cpu_.regs.SP -= 2; cpu_.mem.write16(cpu_.ss_sp(), cpu_.regs.flags.to_word());
                cpu_.regs.flags.IF = false; cpu_.regs.flags.TF = false;
                cpu_.regs.SP -= 2; cpu_.mem.write16(cpu_.ss_sp(), cpu_.regs.CS);
                cpu_.regs.SP -= 2; cpu_.mem.write16(cpu_.ss_sp(), cpu_.regs.IP);
                cpu_.regs.IP = cpu_.mem.read16(vec * 4);
                cpu_.regs.CS = cpu_.mem.read16(vec * 4 + 2);
            }
        } else if (res == ExecResult::DIVISION_BY_ZERO) {
            uint8_t vec = 0;
            cpu_.regs.SP -= 2; cpu_.mem.write16(cpu_.ss_sp(), cpu_.regs.flags.to_word());
            cpu_.regs.flags.IF = false; cpu_.regs.flags.TF = false;
            cpu_.regs.SP -= 2; cpu_.mem.write16(cpu_.ss_sp(), cpu_.regs.CS);
            cpu_.regs.SP -= 2; cpu_.mem.write16(cpu_.ss_sp(), cpu_.regs.IP);
            cpu_.regs.IP = cpu_.mem.read16(vec * 4);
            cpu_.regs.CS = cpu_.mem.read16(vec * 4 + 2);
            res = ExecResult::OK;
        }
    }
    
    if (irq_->halted || res == ExecResult::HALT) state_ = EmulatorState::HALTED;
    else if (res == ExecResult::ILLEGAL_OPCODE) state_ = EmulatorState::ERROR;
}

void Emulator::run_to(uint16_t ip_target) {
    if (state_ == EmulatorState::RUNNING) return;
    if (exec_thread_.joinable()) exec_thread_.join();
    state_ = EmulatorState::RUNNING;
    run_target_ip_ = ip_target;
    exec_thread_ = std::thread(&Emulator::exec_loop_, this);
}

void Emulator::exec_loop_() {
    thread_active_ = true;
    struct ThreadActiveGuard {
        std::atomic<bool>& active;
        ~ThreadActiveGuard() { active = false; }
    } guard{thread_active_};

    auto last_tick = std::chrono::steady_clock::now();
    
    while (state_ == EmulatorState::RUNNING) {
        std::unique_lock<std::mutex> lock(cpu_mutex_);
        
        ExecResult res = executor_->step();
        
        if (res == ExecResult::INTERRUPT_PENDING) {
            if (!irq_->handle(executor_->last_interrupt_num)) {
                uint8_t vec = executor_->last_interrupt_num;
                cpu_.regs.SP -= 2; cpu_.mem.write16(cpu_.ss_sp(), cpu_.regs.flags.to_word());
                cpu_.regs.flags.IF = false; cpu_.regs.flags.TF = false;
                cpu_.regs.SP -= 2; cpu_.mem.write16(cpu_.ss_sp(), cpu_.regs.CS);
                cpu_.regs.SP -= 2; cpu_.mem.write16(cpu_.ss_sp(), cpu_.regs.IP);
                cpu_.regs.IP = cpu_.mem.read16(vec * 4);
                cpu_.regs.CS = cpu_.mem.read16(vec * 4 + 2);
            }
        } else if (res == ExecResult::DIVISION_BY_ZERO) {
            uint8_t vec = 0;
            cpu_.regs.SP -= 2; cpu_.mem.write16(cpu_.ss_sp(), cpu_.regs.flags.to_word());
            cpu_.regs.flags.IF = false; cpu_.regs.flags.TF = false;
            cpu_.regs.SP -= 2; cpu_.mem.write16(cpu_.ss_sp(), cpu_.regs.CS);
            cpu_.regs.SP -= 2; cpu_.mem.write16(cpu_.ss_sp(), cpu_.regs.IP);
            cpu_.regs.IP = cpu_.mem.read16(vec * 4);
            cpu_.regs.CS = cpu_.mem.read16(vec * 4 + 2);
            res = ExecResult::OK;
        }
            
            // Breakpoint/HALT resolution executes AFTER processing the current step 
            // cleanly avoiding resuming issues on top of a breakpoint.
            if (res == ExecResult::HALT || irq_->halted) { state_ = EmulatorState::HALTED; break; }
            if (res == ExecResult::ILLEGAL_OPCODE) { state_ = EmulatorState::ERROR; break; }

        if (res == ExecResult::BREAKPOINT || breakpoints_.count(cpu_.regs.IP)) { state_ = EmulatorState::PAUSED; break; }
        if (run_target_ip_ != 0xFFFF && cpu_.regs.IP == run_target_ip_ && (s_target_cs == 0xFFFF || cpu_.regs.CS == s_target_cs)) {
            state_ = EmulatorState::PAUSED;
            run_target_ip_ = 0xFFFF; // single-shot expiration
            s_target_cs = 0xFFFF;
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
        auto wait_end = now + wait_time;
        while (std::chrono::steady_clock::now() < wait_end) {
            if (state_ != EmulatorState::RUNNING) break;
            auto remaining = std::chrono::duration_cast<std::chrono::microseconds>(wait_end - std::chrono::steady_clock::now());
            if (remaining > std::chrono::microseconds(1000)) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            } else {
                std::this_thread::yield();
            }
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
    std::unique_lock<std::mutex> lock(cpu_mutex_, std::try_to_lock);
    EmulatorSnapshot snap;
    snap.regs = cpu_.regs;
    snap.flags = cpu_.regs.flags;
    snap.cycle_count = executor_->cycle_count;
    if (lock.owns_lock()) {
        if (!irq_->warnings.empty()) snap.last_error = irq_->warnings.back();
    }
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
        std::string line = local_buf.substr(start, end - start);
        if (!line.empty() && line.back() == '\r') line.pop_back();
        lines.push_back(line + '\n'); // Re-attach the newline consumed by split!
        start = end + 1;
        end = local_buf.find('\n', start);
    }
    if (start < local_buf.length()) {
        std::string line = local_buf.substr(start);
        if (!line.empty() && line.back() == '\r') line.pop_back();
        lines.push_back(line);
    }
    
    return lines;
}

} // namespace emu8086::core