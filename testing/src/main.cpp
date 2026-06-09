#include "test_runner.h"
#include <iostream>
#include <filesystem>
#include <vector>
#include <fmt/core.h>
#include <fmt/color.h>

namespace fs = std::filesystem;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: memu_tester <file_or_directory>" << std::endl;
        return 1;
    }

    std::string path = argv[1];
    std::vector<std::string> test_files;

    if (fs::is_directory(path)) {
        for (const auto& entry : fs::directory_iterator(path)) {
            if (entry.path().extension() == ".asm") {
                test_files.push_back(entry.path().string());
            }
        }
    } else if (fs::is_regular_file(path)) {
        test_files.push_back(path);
    } else {
        std::cerr << "Invalid path: " << path << std::endl;
        return 1;
    }

    if (test_files.empty()) {
        std::cout << "No .asm files found to test." << std::endl;
        return 0;
    }

    emu_test::TestRunner runner;
    int passed = 0;
    int failed = 0;

    fmt::print("Running {} tests...\n\n", test_files.size());

    for (const auto& file : test_files) {
        auto result = runner.run_file(file);
        if (result.success) {
            passed++;
            fmt::print(fg(fmt::color::green), "[ PASS ] ");
            fmt::print("{} ({} cycles)\n", file, result.cycles_executed);
        } else {
            failed++;
            fmt::print(fg(fmt::color::red), "[ FAIL ] ");
            fmt::print("{}\n", file);
            for (const auto& err : result.errors) {
                fmt::print(fg(fmt::color::red), "         {}\n", err);
            }
        }
    }

    fmt::print("\n------------------------------------------\n");
    fmt::print("Summary: {} passed, {} failed\n", passed, failed);

    return (failed == 0) ? 0 : 1;
}
