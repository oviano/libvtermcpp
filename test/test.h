/*
 * test.h — zero-dependency single-header test framework for libvtermcpp
 *
 * Usage:
 *   #include "test.h"
 *   TEST(my_test) { ASSERT_EQ(1, 1); }
 *
 * Tests are auto-registered via constructor attribute.
 * main.cpp calls test_run_all() which runs them and prints a summary.
 */

#ifndef TEST_H
#define TEST_H

#include <array>
#include <cstdlib>
#include <format>
#include <iostream>
#include <string_view>

// --- Test registration ---

using test_fn = void(*)(int32_t* _test_failures);

struct test_entry {
    std::string_view test_name{};
    std::string_view test_file{};
    test_fn fn = nullptr;
};

inline constexpr int32_t TEST_MAX = 4096;

// Global test registry — defined in main.cpp
extern std::array<test_entry, TEST_MAX> g_tests;
extern int32_t g_test_count;

#define TEST(name)                                                          \
    static void test_##name([[maybe_unused]] int32_t *_test_failures);       \
    __attribute__((constructor)) static void test_register_##name() {        \
        g_tests[g_test_count].test_name = #name;                            \
        g_tests[g_test_count].test_file = __FILE__;                         \
        g_tests[g_test_count].fn = test_##name;                             \
        g_test_count++;                                                     \
    }                                                                       \
    static void test_##name([[maybe_unused]] int32_t *_test_failures)

// --- Assertions ---

#define ASSERT_TRUE(expr)                                                   \
    do {                                                                    \
        if (!(expr)) {                                                      \
            std::cerr << std::format("  FAIL {}:{}: ASSERT_TRUE({})\n",     \
                    __FILE__, __LINE__, #expr);                              \
            (*_test_failures)++;                                             \
            return;                                                         \
        }                                                                   \
    } while (0)

#define ASSERT_EQ(a, b)                                                     \
    do {                                                                    \
        int64_t _a = static_cast<int64_t>(a), _b = static_cast<int64_t>(b); \
        if (_a != _b) {                                                     \
            std::cerr << std::format("  FAIL {}:{}: ASSERT_EQ({}, {}) — got {}, expected {}\n", \
                    __FILE__, __LINE__, #a, #b, _a, _b);                    \
            (*_test_failures)++;                                             \
            return;                                                         \
        }                                                                   \
    } while (0)

#define ASSERT_NEQ(a, b)                                                    \
    do {                                                                    \
        int64_t _a = static_cast<int64_t>(a), _b = static_cast<int64_t>(b); \
        if (_a == _b) {                                                     \
            std::cerr << std::format("  FAIL {}:{}: ASSERT_NEQ({}, {}) — both are {}\n", \
                    __FILE__, __LINE__, #a, #b, _a);                        \
            (*_test_failures)++;                                             \
            return;                                                         \
        }                                                                   \
    } while (0)

#define ASSERT_STR_EQ(a, b)                                                 \
    do {                                                                    \
        const char *_a = (a), *_b = (b);                                    \
        if (!_a || !_b || std::string_view{_a} != std::string_view{_b}) {   \
            std::cerr << std::format("  FAIL {}:{}: ASSERT_STR_EQ({}, {}) — got \"{}\", expected \"{}\"\n", \
                    __FILE__, __LINE__, #a, #b, _a ? _a : "(null)", _b ? _b : "(null)"); \
            (*_test_failures)++;                                             \
            return;                                                         \
        }                                                                   \
    } while (0)

// --- Test runner (called from main.cpp) ---

inline int test_run_all(int argc, char** argv)
{
    std::string_view filter = (argc > 1) ? std::string_view{argv[1]} : std::string_view{};
    int32_t total = 0, passed = 0, failed = 0, skipped = 0;

    for (int32_t i = 0; i < g_test_count; i++) {
        if (!filter.empty() && g_tests[i].test_name.find(filter) == std::string_view::npos) {
            skipped++;
            continue;
        }

        total++;

        int32_t failures = 0;
        g_tests[i].fn(&failures);
        if (failures == 0) {
            passed++;
        } else {
            std::cerr << std::format("  FAILED: {} ({})\n", g_tests[i].test_name, g_tests[i].test_file);
            failed++;
        }
    }

    std::cout << std::format("\n{} tests: {} passed, {} failed", total, passed, failed);
    if (skipped > 0)
        std::cout << std::format(", {} skipped", skipped);
    std::cout << "\n";

    return failed > 0 ? 1 : 0;
}

#endif // TEST_H
