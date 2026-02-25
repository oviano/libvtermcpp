// test_scrollback.cpp -- built-in scrollback storage, reflow, and resize compensation
//
// Unit tests exercise Scrollback::Impl directly (constructed standalone).
// Integration tests exercise Screen + Scrollback working together via Terminal.
// Stress tests write large output, resize, and verify against golden files.

#include "harness.h"
#include "../src/scrollback_impl.h"

#include <array>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <random>
#include <sstream>
#include <string>

// ============================================================================
// Helpers
// ============================================================================

// Helper: make a ScreenCell with a single character
static ScreenCell make_cell(uint32_t ch, int32_t width = 1) {
    ScreenCell cell{};
    cell.chars[0] = ch;
    cell.width = width;
    return cell;
}

// Helper: make a row of cells from a string
static std::vector<ScreenCell> make_row(std::string_view text, int32_t padto = 0) {
    std::vector<ScreenCell> cells;
    for(char c : text)
        cells.push_back(make_cell(static_cast<uint32_t>(c)));
    while(static_cast<int32_t>(cells.size()) < padto)
        cells.push_back(make_cell(0));
    return cells;
}

// Helper: extract text from scrollback line (strips trailing blanks)
static std::string sb_line_text(const Scrollback::Line& line) {
    std::string s;
    for(const auto& cell : line.cells) {
        if(cell.chars[0] == 0) break;
        if(cell.chars[0] < 128)
            s += static_cast<char>(cell.chars[0]);
    }
    return s;
}

// Standard setup for integration tests using built-in scrollback
#define SB_SETUP(rows, cols, cap) \
    Terminal vt(25, 80); \
    vt.set_utf8(false); \
    State& state = vt.state(); \
    state.set_callbacks(state_cbs_no_scrollrect); \
    state.reset(true); \
    Screen& screen = vt.screen(); \
    Scrollback& sb = vt.scrollback(); \
    sb.set_capacity(cap); \
    screen.enable_reflow(true); \
    vt.set_size((rows), (cols)); \
    screen.reset(true)

// ============================================================================
// Golden file helpers
// ============================================================================

static std::string golden_dir() {
    return std::string(TEST_GOLDEN_DIR);
}

static std::string capture_state(Terminal& vt, Scrollback& sb) {
    std::ostringstream os;

    // Scrollback
    os << "[scrollback:" << sb.size() << "]\n";
    for(size_t i = 0; i < sb.size(); i++) {
        const auto& line = sb.line(i);
        os << i << ": |";
        for(const auto& cell : line.cells) {
            if(cell.chars[0] == 0)
                os << ' ';
            else if(cell.chars[0] < 128)
                os << static_cast<char>(cell.chars[0]);
            else
                os << '?';
        }
        os << "| cont=" << (line.continuation ? 1 : 0) << "\n";
    }

    // Screen
    int32_t rows = vt.rows(), cols = vt.cols();
    os << "[screen:" << rows << "x" << cols << "]\n";
    Screen& screen = vt.screen();
    for(int32_t r = 0; r < rows; r++) {
        os << r << ": |";
        for(int32_t c = 0; c < cols; c++) {
            ScreenCell cell{};
            (void)screen.get_cell({r, c}, cell);
            if(cell.chars[0] == 0)
                os << ' ';
            else if(cell.chars[0] < 128)
                os << static_cast<char>(cell.chars[0]);
            else
                os << '?';
        }
        os << "|\n";
    }

    // Cursor
    Pos cursor = vt.state().cursor_pos();
    os << "[cursor:" << cursor.row << "," << cursor.col << "]\n";

    return os.str();
}

static bool is_generate_mode() {
    const char* env = std::getenv("GENERATE_GOLDEN");
    return env && std::string_view(env) == "1";
}

static void assert_golden(int32_t* _test_failures, const std::string& test_name,
                           int step, const std::string& actual) {
    std::string filename = golden_dir() + "/" + test_name + "_" + std::to_string(step) + ".txt";

    if(is_generate_mode()) {
        std::filesystem::create_directories(golden_dir());
        std::ofstream f(filename);
        f << actual;
        return;
    }

    std::ifstream f(filename);
    if(!f.is_open()) {
        std::cerr << "  FAIL: golden file not found: " << filename << "\n";
        (*_test_failures)++;
        return;
    }

    std::string expected((std::istreambuf_iterator<char>(f)),
                          std::istreambuf_iterator<char>());

    if(actual != expected) {
        std::cerr << "  FAIL: golden mismatch: " << filename << "\n";
        // Show first difference
        for(size_t i = 0; i < std::min(actual.size(), expected.size()); i++) {
            if(actual[i] != expected[i]) {
                std::cerr << "  First diff at byte " << i << ": got '"
                          << actual[i] << "' expected '" << expected[i] << "'\n";
                break;
            }
        }
        if(actual.size() != expected.size())
            std::cerr << "  Size: got " << actual.size() << " expected " << expected.size() << "\n";
        (*_test_failures)++;
    }
}

// ============================================================================
// Unit tests: Scrollback::Impl in isolation
// ============================================================================

TEST(scrollback_push_pop) {
    Scrollback::Impl impl;
    impl.capacity = 100;

    impl.push_line(make_row("AAAA", 10), false);
    impl.push_line(make_row("BBBB", 10), true);
    impl.push_line(make_row("CCCC", 10), false);

    ASSERT_EQ(impl.lines.size(), 3);

    // Pop in LIFO order
    std::vector<ScreenCell> buf(10);
    bool cont = false;

    ASSERT_TRUE(impl.pop_line(buf, cont));
    ASSERT_EQ(buf[0].chars[0], 'C');
    ASSERT_EQ(cont, false);

    ASSERT_TRUE(impl.pop_line(buf, cont));
    ASSERT_EQ(buf[0].chars[0], 'B');
    ASSERT_EQ(cont, true);

    ASSERT_TRUE(impl.pop_line(buf, cont));
    ASSERT_EQ(buf[0].chars[0], 'A');
    ASSERT_EQ(cont, false);

    ASSERT_TRUE(impl.lines.empty());
    ASSERT_TRUE(!impl.pop_line(buf, cont));
}

TEST(scrollback_capacity_eviction) {
    Scrollback::Impl impl;
    impl.capacity = 5;

    for(int i = 0; i < 8; i++) {
        auto row = make_row(std::string(1, static_cast<char>('A' + i)), 10);
        impl.push_line(row, false);
    }

    // Should have evicted oldest 3 (A, B, C)
    ASSERT_EQ(impl.lines.size(), 5);
    ASSERT_EQ(impl.lines[0].cells[0].chars[0], 'D'); // oldest remaining
    ASSERT_EQ(impl.lines[4].cells[0].chars[0], 'H'); // newest
}

TEST(scrollback_clear) {
    Scrollback::Impl impl;
    impl.capacity = 100;

    impl.push_line(make_row("A", 10), false);
    impl.push_line(make_row("B", 10), false);
    ASSERT_EQ(impl.lines.size(), 2);

    impl.clear();
    ASSERT_TRUE(impl.lines.empty());
}

TEST(scrollback_line_access) {
    Scrollback::Impl impl;
    impl.capacity = 100;

    impl.push_line(make_row("FIRST", 10), false);
    impl.push_line(make_row("MIDDLE", 10), true);
    impl.push_line(make_row("LAST", 10), false);

    // [0] = oldest
    ASSERT_EQ(impl.lines[0].cells[0].chars[0], 'F');
    ASSERT_EQ(impl.lines[0].continuation, false);

    // [1] = middle
    ASSERT_EQ(impl.lines[1].cells[0].chars[0], 'M');
    ASSERT_EQ(impl.lines[1].continuation, true);

    // [2] = newest
    ASSERT_EQ(impl.lines[2].cells[0].chars[0], 'L');
    ASSERT_EQ(impl.lines[2].continuation, false);
}

TEST(scrollback_reflow_wider) {
    Scrollback::Impl impl;
    impl.capacity = 100;

    // "ABCDE" + "FGH" (continuation) = logical line "ABCDEFGH"
    impl.push_line(make_row("ABCDE", 5), false);
    impl.push_line(make_row("FGH", 5), true);

    ASSERT_EQ(impl.lines.size(), 2);

    impl.reflow(10);

    // Should be single line now
    ASSERT_EQ(impl.lines.size(), 1);
    ASSERT_EQ(impl.lines[0].continuation, false);
    ASSERT_TRUE(sb_line_text(impl.lines[0]) == "ABCDEFGH");
}

TEST(scrollback_reflow_narrower) {
    Scrollback::Impl impl;
    impl.capacity = 100;

    impl.push_line(make_row("ABCDEFGH", 10), false);

    impl.reflow(4);

    // Should be 2 lines: "ABCD" + "EFGH" (continuation)
    ASSERT_EQ(impl.lines.size(), 2);
    ASSERT_EQ(impl.lines[0].continuation, false);
    ASSERT_EQ(impl.lines[1].continuation, true);
    ASSERT_TRUE(sb_line_text(impl.lines[0]) == "ABCD");
    ASSERT_TRUE(sb_line_text(impl.lines[1]) == "EFGH");
}

TEST(scrollback_reflow_empty_line) {
    Scrollback::Impl impl;
    impl.capacity = 100;

    impl.push_line(make_row("", 10), false);
    impl.push_line(make_row("HELLO", 10), false);

    impl.reflow(5);

    ASSERT_EQ(impl.lines.size(), 2);
    ASSERT_TRUE(sb_line_text(impl.lines[0]).empty());
    ASSERT_EQ(impl.lines[0].continuation, false);
    ASSERT_TRUE(sb_line_text(impl.lines[1]) == "HELLO");
}

TEST(scrollback_resize_comp_shrink_grow) {
    Scrollback::Impl impl;
    impl.capacity = 100;

    impl.begin_resize();
    impl.push_line(make_row("PUSHED1", 10), false);
    impl.push_line(make_row("PUSHED2", 10), false);
    impl.commit_resize(4, 2, 10, 10);

    ASSERT_EQ(impl.lines.size(), 2);

    // Grow back
    impl.begin_resize();
    impl.commit_resize(2, 4, 10, 10);

    // Tracked lines should be erased
    ASSERT_EQ(impl.lines.size(), 0);
}

TEST(scrollback_resize_comp_width_change_resets) {
    Scrollback::Impl impl;
    impl.capacity = 100;

    impl.begin_resize();
    impl.push_line(make_row("A", 10), false);
    impl.commit_resize(4, 3, 10, 10);

    ASSERT_EQ(impl.push_track_count, 1);

    // Resize with column change — tracking should reset
    impl.begin_resize();
    impl.commit_resize(3, 3, 10, 20);

    ASSERT_EQ(impl.push_track_count, 0);
    ASSERT_EQ(impl.push_track_start, 0);
}

TEST(scrollback_resize_comp_capacity_eviction) {
    Scrollback::Impl impl;
    impl.capacity = 3;

    impl.push_line(make_row("OLD1", 10), false);
    impl.push_line(make_row("OLD2", 10), false);
    impl.push_line(make_row("OLD3", 10), false);
    ASSERT_EQ(impl.lines.size(), 3);

    // Begin tracking, push more (triggers eviction)
    impl.begin_resize();
    impl.push_line(make_row("NEW1", 10), false);
    ASSERT_EQ(impl.lines.size(), 3);

    impl.push_line(make_row("NEW2", 10), false);
    ASSERT_EQ(impl.lines.size(), 3);

    impl.commit_resize(4, 2, 10, 10);

    ASSERT_TRUE(impl.lines.size() <= 3);
}

// ============================================================================
// Integration tests: Screen + Scrollback
// ============================================================================

TEST(scrollback_screen_scroll) {
    SB_SETUP(4, 10, 100);

    push(vt, "LINE1\r\n");
    push(vt, "LINE2\r\n");
    push(vt, "LINE3\r\n");
    push(vt, "LINE4\r\n");
    push(vt, "LINE5");

    ASSERT_EQ(sb.size(), 1);
    ASSERT_TRUE(sb_line_text(sb.line(0)) == "LINE1");

    push(vt, "\r\nLINE6\r\nLINE7");

    ASSERT_EQ(sb.size(), 3);
    ASSERT_TRUE(sb_line_text(sb.line(0)) == "LINE1");
    ASSERT_TRUE(sb_line_text(sb.line(1)) == "LINE2");
    ASSERT_TRUE(sb_line_text(sb.line(2)) == "LINE3");
}

TEST(scrollback_screen_resize_pop) {
    SB_SETUP(4, 10, 100);

    push(vt, "LINE1\r\n");
    push(vt, "LINE2\r\n");
    push(vt, "LINE3\r\n");
    push(vt, "LINE4\r\n");
    push(vt, "LINE5\r\n");
    push(vt, "LINE6");

    ASSERT_EQ(sb.size(), 2);
    ASSERT_SCREEN_ROW(vt, screen, 0, "LINE3");

    // Grow terminal height — should pop from scrollback
    vt.set_size(6, 10);

    ASSERT_EQ(sb.size(), 0);
    ASSERT_SCREEN_ROW(vt, screen, 0, "LINE1");
    ASSERT_SCREEN_ROW(vt, screen, 5, "LINE6");
}

TEST(scrollback_screen_reflow_roundtrip) {
    SB_SETUP(4, 10, 100);

    push(vt, "SHORT\r\n");
    push(vt, "AAAAABBBBBCCC\r\n");
    push(vt, "DD\r\n");
    push(vt, "EE");

    ASSERT_EQ(sb.size(), 1);
    ASSERT_TRUE(sb_line_text(sb.line(0)) == "SHORT");

    // Resize wider: wrapped line joins, freeing row for scrollback pop
    vt.set_size(4, 15);

    ASSERT_EQ(sb.size(), 0);
    ASSERT_SCREEN_ROW(vt, screen, 0, "SHORT");
    ASSERT_SCREEN_ROW(vt, screen, 1, "AAAAABBBBBCCC");
}

TEST(scrollback_screen_altscreen_no_push) {
    SB_SETUP(4, 10, 100);

    push(vt, "LINE1\r\n");
    push(vt, "LINE2\r\n");
    push(vt, "LINE3\r\n");
    push(vt, "LINE4");

    size_t sb_before = sb.size();

    // Switch to altscreen
    screen.enable_altscreen(true);
    push(vt, "\x1b[?1049h");

    // Write in altscreen — should NOT push to scrollback
    push(vt, "ALT1\r\n");
    push(vt, "ALT2\r\n");
    push(vt, "ALT3\r\n");
    push(vt, "ALT4\r\n");
    push(vt, "ALT5");

    ASSERT_EQ(sb.size(), sb_before);

    // Switch back
    push(vt, "\x1b[?1049l");
}

TEST(scrollback_screen_callback_still_fires) {
    SB_SETUP(4, 10, 100);

    // Also register a user callback
    callbacks_clear();
    screen.set_callbacks(screen_cbs_scrollback_reflow);
    scrollback_clear();

    push(vt, "LINE1\r\n");
    push(vt, "LINE2\r\n");
    push(vt, "LINE3\r\n");
    push(vt, "LINE4\r\n");
    push(vt, "LINE5");

    // Built-in scrollback should have the line
    ASSERT_EQ(sb.size(), 1);
    ASSERT_TRUE(sb_line_text(sb.line(0)) == "LINE1");

    // Callback-based scrollback should also have it
    ASSERT_EQ(g_scrollback_count, 1);
    ASSERT_EQ(g_scrollback[0].cells[0].chars[0], 'L');
}

// ============================================================================
// Stress tests with golden output files
// ============================================================================

TEST(scrollback_stress_large_output) {
    Terminal vt(24, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs_no_scrollrect);
    state.reset(true);
    Screen& screen = vt.screen();
    Scrollback& sb = vt.scrollback();
    sb.set_capacity(1000);
    screen.enable_reflow(true);

    // Write 5000 lines of varying length
    std::mt19937 rng(42);
    for(int i = 0; i < 5000; i++) {
        int len = 5 + static_cast<int>(rng() % 196);
        std::string line;
        for(int j = 0; j < len; j++)
            line += static_cast<char>('A' + (j % 26));
        line += "\r\n";
        push(vt, line);
    }

    ASSERT_TRUE(sb.size() <= 1000);
    ASSERT_TRUE(sb.size() > 0);

    assert_golden(_test_failures, "scrollback_stress_large_output", 0,
                  capture_state(vt, sb));
    if(*_test_failures) return;

    constexpr std::array widths = {40, 120, 60, 80};
    for(size_t step = 0; step < widths.size(); step++) {
        vt.set_size(24, widths[step]);
        assert_golden(_test_failures, "scrollback_stress_large_output", static_cast<int>(step) + 1,
                      capture_state(vt, sb));
        if(*_test_failures) return;
    }
}

TEST(scrollback_stress_rapid_resize) {
    Terminal vt(24, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs_no_scrollrect);
    state.reset(true);
    Screen& screen = vt.screen();
    Scrollback& sb = vt.scrollback();
    sb.set_capacity(500);
    screen.enable_reflow(true);

    for(int i = 0; i < 1000; i++) {
        std::string line = "Line" + std::to_string(i);
        while(static_cast<int>(line.size()) < 20 + (i % 60))
            line += static_cast<char>('a' + (i % 26));
        line += "\r\n";
        push(vt, line);
    }

    struct ResizeStep { int rows; int cols; };
    constexpr std::array<ResizeStep, 20> steps = {{
        {10, 40}, {50, 120}, {24, 60}, {15, 200}, {40, 30},
        {24, 80}, {10, 150}, {50, 20}, {24, 100}, {15, 80},
        {40, 50}, {24, 160}, {10, 45}, {50, 90}, {24, 35},
        {15, 110}, {40, 70}, {24, 80}, {10, 80}, {24, 80}
    }};

    for(size_t step = 0; step < steps.size(); step++) {
        vt.set_size(steps[step].rows, steps[step].cols);
        assert_golden(_test_failures, "scrollback_stress_rapid_resize", static_cast<int>(step),
                      capture_state(vt, sb));
        if(*_test_failures) return;
    }
}

TEST(scrollback_stress_interleaved) {
    Terminal vt(24, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs_no_scrollrect);
    state.reset(true);
    Screen& screen = vt.screen();
    Scrollback& sb = vt.scrollback();
    sb.set_capacity(500);
    screen.enable_reflow(true);

    std::mt19937 rng(123);

    for(int cycle = 0; cycle < 10; cycle++) {
        for(int i = 0; i < 100; i++) {
            int len = 10 + static_cast<int>(rng() % 100);
            std::string line;
            for(int j = 0; j < len; j++)
                line += static_cast<char>('A' + (j % 26));
            line += "\r\n";
            push(vt, line);
        }

        int new_cols = 20 + static_cast<int>(rng() % 180);
        int new_rows = 10 + static_cast<int>(rng() % 40);
        vt.set_size(new_rows, new_cols);

        assert_golden(_test_failures, "scrollback_stress_interleaved", cycle,
                      capture_state(vt, sb));
        if(*_test_failures) return;
    }
}

TEST(scrollback_stress_wide_chars) {
    Terminal vt(24, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs_no_scrollrect);
    state.reset(true);
    Screen& screen = vt.screen();
    Scrollback& sb = vt.scrollback();
    sb.set_capacity(500);
    screen.enable_reflow(true);

    for(int i = 0; i < 200; i++) {
        std::string line;
        for(int j = 0; j < 20; j++) {
            if(j % 3 == 0) {
                uint32_t cp = 0x4E00 + (j % 10);
                line += static_cast<char>(0xE0 | ((cp >> 12) & 0x0F));
                line += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
                line += static_cast<char>(0x80 | (cp & 0x3F));
            }
            else {
                line += static_cast<char>('A' + (j % 26));
            }
        }
        line += "\r\n";
        push(vt, line);
    }

    assert_golden(_test_failures, "scrollback_stress_wide_chars", 0,
                  capture_state(vt, sb));
    if(*_test_failures) return;

    constexpr std::array widths = {40, 79, 41, 120, 33, 80};
    for(size_t step = 0; step < widths.size(); step++) {
        vt.set_size(24, widths[step]);
        assert_golden(_test_failures, "scrollback_stress_wide_chars", static_cast<int>(step) + 1,
                      capture_state(vt, sb));
        if(*_test_failures) return;
    }
}
