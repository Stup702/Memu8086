#pragma once

#include <string>
#include <vector>
#include <map>
#include <cstdint>

namespace emu_test {

struct Expectation {
    enum class Type { REGISTER, FLAG, MEMORY, ASM_ERROR };
    Type type;
    std::string name; // Register or flag name
    uint32_t address; // For memory
    uint16_t expected_value;
};

struct TestResult {
    bool success;
    std::string test_name;
    std::vector<std::string> errors;
    uint32_t cycles_executed;
};

class TestRunner {
public:
    TestRunner();
    TestResult run_file(const std::string& file_path);

private:
    std::vector<Expectation> parse_expectations(const std::string& source);
};

} // namespace emu_test
