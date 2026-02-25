// main.cpp — test runner for libvtermcpp

#include "test.h"

std::array<test_entry, TEST_MAX> g_tests{};
int32_t g_test_count = 0;

int main(int argc, char** argv) {
    return test_run_all(argc, argv);
}
