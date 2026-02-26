// test_93_regression_review.cpp — regression tests for code review findings #1-14
//
// Each test targets a specific bug found during review that was not caught by
// the existing test suite. The bugs involve combinations of features that were
// individually well-tested but never tested together.
//
// Tests assert correctness properties (correct data, valid coordinates, no
// split wide characters) rather than just "doesn't crash", so they catch bugs
// through observable data errors. Bugs that corrupt memory (e.g. SIZE_MAX span
// from an underflow) will crash the test process — also a detected failure.

#include "harness.h"
#include "../src/scrollback_impl.h"

// Helper: make a ScreenCell with a single character
static ScreenCell make_cell(uint32_t ch, int32_t width = 1) {
    ScreenCell cell{};
    cell.chars[0] = ch;
    cell.width = width;
    return cell;
}

// Helper: make a row of cells from a string, padded to width
static std::vector<ScreenCell> make_row(std::string_view text, int32_t padto = 0) {
    std::vector<ScreenCell> cells;
    for(char c : text)
        cells.push_back(make_cell(static_cast<uint32_t>(c)));
    while(static_cast<int32_t>(cells.size()) < padto)
        cells.push_back(make_cell(0));
    return cells;
}

// Helper: assert no scrollback row has a width-2 cell at its last column.
static bool assert_no_split_wide_chars_sb(int32_t* _test_failures,
                                           const Scrollback::Impl& impl,
                                           int32_t cols) {
    for(size_t i = 0; i < impl.lines.size(); i++) {
        const auto& line = impl.lines[i];
        if(static_cast<int32_t>(line.cells.size()) >= cols &&
           line.cells[static_cast<size_t>(cols - 1)].width == 2) {
            std::cerr << std::format("  FAIL {}:{}: scrollback row {}: width-2 cell at last column {}\n",
                                      __FILE__, __LINE__, i, cols - 1);
            (*_test_failures)++;
            return false;
        }
    }
    return true;
}

// Helper: assert no screen row has an orphaned widechar continuation marker
// (0xFFFFFFFF) at col 0. This indicates a double-width character was split
// across rows — its right half landed at the start of the next row.
static constexpr uint32_t widechar_continuation_marker = 0xFFFFFFFF;

static bool assert_no_orphan_wide_chars(int32_t* _test_failures, Screen& screen,
                                         int32_t rows, int32_t cols) {
    for(int32_t r = 0; r < rows; r++) {
        // Check for orphan continuation at col 0
        ScreenCell cell{};
        (void)screen.get_cell({r, 0}, cell);
        if(cell.chars[0] == widechar_continuation_marker) {
            std::cerr << std::format("  FAIL {}:{}: orphan widechar continuation at ({},0)\n",
                                      __FILE__, __LINE__, r);
            (*_test_failures)++;
            return false;
        }

        // Check for a CJK char at the last column (it needs 2 cols but has only 1).
        // The screen's get_cell reports width=1 for such cells (no next cell to check),
        // so we detect it by the character value: CJK chars (> U+2E80) at last column
        // with width=1 indicate a split.
        ScreenCell last{};
        (void)screen.get_cell({r, cols - 1}, last);
        if(last.chars[0] > 0x2E80 && last.width == 1) {
            std::cerr << std::format("  FAIL {}:{}: CJK U+{:04X} at ({},{}), last column (split)\n",
                                      __FILE__, __LINE__, last.chars[0], r, cols - 1);
            (*_test_failures)++;
            return false;
        }
    }
    return true;
}

// Helper: collect all text from scrollback into a string
static std::string collect_scrollback_text(const Scrollback& sb) {
    std::string text;
    for(size_t i = 0; i < sb.size(); i++) {
        const auto& line = sb.line(i);
        for(const auto& cell : line.cells) {
            if(cell.chars[0] == 0) break;
            if(cell.chars[0] < 128)
                text += static_cast<char>(cell.chars[0]);
        }
    }
    return text;
}

// Helper: collect all text from screen into a string
static std::string collect_screen_text(Terminal& vt, Screen& screen) {
    std::string text;
    for(int32_t r = 0; r < vt.rows(); r++) {
        for(int32_t c = 0; c < vt.cols(); c++) {
            ScreenCell cell{};
            (void)screen.get_cell({r, c}, cell);
            if(cell.chars[0] == 0) break;
            if(cell.chars[0] < 128)
                text += static_cast<char>(cell.chars[0]);
        }
    }
    return text;
}

// ============================================================================
// Group 1: Parser split-write at state boundaries (#1, #7)
// ============================================================================

// #1: write() ending with ESC ] enters OSCCommand state where string_start is
// SIZE_MAX. Next write() starting with NUL calls string_fragment with an
// invalid span. We verify the parser delivers a correct OSC after the split.
TEST(regression_parser_osc_split_nul)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Enter OSCCommand state — string_start is SIZE_MAX
    push(vt, "\e]");

    // NUL in a separate write — the bug would OOB here
    push(vt, std::string_view("\0", 1));

    // Continue with OSC command number + data + terminator
    push(vt, "2;TestTitle\a");

    // Verify the OSC was delivered correctly with the right content
    bool found_title = false;
    for(int32_t i = 0; i < g_cb.settermprop_count; i++) {
        if(g_cb.settermprop[i].prop == Prop::Title &&
           g_cb.settermprop[i].val.string.final_) {
            found_title = true;
            break;
        }
    }
    ASSERT_TRUE(found_title);
}

// #1 variant: C0 control (LF) after split OSC start. The LF should be
// dispatched as a control, and the OSC should still complete correctly.
TEST(regression_parser_osc_split_c0)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);

    // Move cursor to row 0 col 0 as baseline
    push(vt, "\e[1;1H");
    callbacks_clear();

    // Enter OSCCommand state
    push(vt, "\e]");

    // LF in a separate write — the bug would OOB here.
    // The LF should be processed as a C0 control (cursor moves down).
    push(vt, "\n");

    // The LF should have moved the cursor down one row
    ASSERT_CURSOR(state, 1, 0);

    // Complete the OSC and verify parser is functional
    push(vt, "2;AfterLF\a");

    bool found_title = false;
    for(int32_t i = 0; i < g_cb.settermprop_count; i++) {
        if(g_cb.settermprop[i].prop == Prop::Title &&
           g_cb.settermprop[i].val.string.final_) {
            found_title = true;
            break;
        }
    }
    ASSERT_TRUE(found_title);
}

// #7: string_len underflow when in_esc is true at end of buffer with
// string_len == 0. The trailing code does string_len -= 1, wrapping to
// SIZE_MAX. This creates a subspan that crashes or corrupts data.
//
// Trigger: write 1 puts parser in OSC state ending with ESC (in_esc=true).
// Write 2 is a single C0 control — the C0 handler updates string_start to
// pos+1 (== data.size()), so at end of loop string_len = pos - string_start
// = 0. The unguarded decrement wraps to SIZE_MAX.
TEST(regression_parser_string_len_no_underflow)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    vt.parser_set_callbacks(parser_cbs);
    parser_clear();

    // Write 1: Enter OSC 2, send "Hello", end with ESC (in_esc=true).
    // Trailing code correctly sends "Hello" fragment (string_len = 6 - 1 = 5).
    push(vt, "\e]2;Hello\e");

    // Write 2: C0 control in string state with in_esc still true.
    // C0 handler sets string_start = 1. End of loop: string_len = 1-1 = 0.
    // Bug: 0 - 1 = SIZE_MAX → subspan(1, SIZE_MAX) → crash.
    // Fix: guarded by string_len > 0, empty fragment sent instead.
    push(vt, "\n");

    // Write 3: Complete ST (backslash after previous ESC).
    push(vt, "\\");

    // Verify OSC was received correctly
    ASSERT_EQ(g_parser.osc_count, 1);
    ASSERT_EQ(g_parser.osc[0].command, 2);
    ASSERT_TRUE(g_parser.osc[0].final_);
    ASSERT_EQ(g_parser.osc[0].datalen, 5);
    ASSERT_TRUE(std::string_view(g_parser.osc[0].data.data(), 5) == "Hello");
}

// ============================================================================
// Group 2: Wide chars + reflow/resize (#8, #10, #11, #12)
// ============================================================================

// #8: Scrollback reflow splits double-width characters across row boundaries.
// Unit test on Scrollback::Impl — the wide char at offset 4 in a 5-col reflow
// would land at the last column with its continuation on the next row.
TEST(regression_wide_char_scrollback_reflow)
{
    Scrollback::Impl impl;
    impl.capacity = 100;

    // Build: 4 ASCII + wide(2) + 4 ASCII = 10 cells
    // Column positions:  0 1 2 3  4,5  6 7 8 9
    std::vector<ScreenCell> row;
    for(int i = 0; i < 4; i++)
        row.push_back(make_cell('A' + i));
    row.push_back(make_cell(0x4E00, 2));  // width-2 CJK at offset 4
    row.push_back(make_cell(0, 0));       // continuation
    for(int i = 0; i < 4; i++)
        row.push_back(make_cell('E' + i));

    impl.push_line(std::span<const ScreenCell>(row), false);

    // Reflow to width 5 — offset 4 is the last column
    impl.reflow(5);

    if(!assert_no_split_wide_chars_sb(_test_failures, impl, 5))
        return;

    // Verify all content survived
    bool found_cjk = false;
    int ascii_count = 0;
    for(const auto& line : impl.lines) {
        for(const auto& cell : line.cells) {
            if(cell.chars[0] == 0x4E00) found_cjk = true;
            if(cell.chars[0] >= 'A' && cell.chars[0] <= 'H') ascii_count++;
        }
    }
    ASSERT_TRUE(found_cjk);
    ASSERT_EQ(ascii_count, 8);
}

// #10, #11: Screen reflow pulls content from scrollback. The wide char at
// col 4 of a 10-col scrollback line maps to col 4 (last column) in a 5-col
// reflow. Bug #11: height miscalculated. Bug #10: wide char split at boundary.
//
// Setup: 2-row terminal so writing 2+ lines pushes content to scrollback.
// The scrollback line has CJK at col 4. Resize to more rows (triggers backfill)
// and 5 cols (CJK at col 4 = last column). The screen-resident content is
// plain ASCII so the main loop doesn't produce its own split.
TEST(regression_wide_char_screen_reflow)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs_no_scrollrect);
    state.reset(true);
    Screen& screen = vt.screen();
    Scrollback& sb = vt.scrollback();
    sb.set_capacity(100);
    screen.enable_reflow(true);
    vt.set_size(2, 10);
    screen.reset(true);

    // Line 1: CJK at col 4 — this will be pushed to scrollback
    push(vt, "AAAA\xE4\xB8\x80""BBBB\r\n");
    // Line 2: plain ASCII — stays on screen row 0 after scroll
    push(vt, "XXXX\r\n");
    // Line 3: cursor on row 1
    push(vt, "YY");

    // Scrollback now has "AAAA一BBBB" (10 cells, CJK at col 4).
    // Screen: row 0 = "XXXX", row 1 = "YY".

    // Resize to 6 rows, 5 cols. The extra rows trigger scrollback backfill.
    // During resize_buffer, the scrollback hasn't been reflowed yet
    // (commit_resize runs after), so the backfill sees the original 10-col line.
    // CJK at col 4 maps to col 4 = last column of 5-col row.
    vt.set_size(6, 5);

    if(!assert_no_orphan_wide_chars(_test_failures, screen, vt.rows(), vt.cols()))
        return;

    // Verify text content survived
    std::string all_text = collect_scrollback_text(sb);
    all_text += collect_screen_text(vt, screen);
    ASSERT_TRUE(all_text.find("AAAA") != std::string::npos);
    ASSERT_TRUE(all_text.find("XXXX") != std::string::npos);
}

// #12: Non-reflow popline path. Content popped from scrollback at original
// (wider) width is truncated to new_cols. CJK at col 3 of a 10-col line
// maps to col 3 (last column) of a 4-col terminal. Bug: CJK placed at last
// column with no room for its right half.
TEST(regression_wide_char_resize_truncate)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs_no_scrollrect);
    state.reset(true);
    Screen& screen = vt.screen();
    Scrollback& sb = vt.scrollback();
    sb.set_capacity(100);
    screen.enable_reflow(false);  // non-reflow path
    vt.set_size(2, 10);
    screen.reset(true);

    // Line with CJK at col 3: 3 ASCII + CJK(2) + 5 ASCII = 10 cells
    push(vt, "AAA\xE4\xB8\x80""DDDDD\r\n");
    // Plain ASCII line
    push(vt, "XX\r\n");
    push(vt, "YY");

    // Scrollback has "AAA一DDDDD". Screen: "XX", "YY".

    // Resize to 4 rows, 4 cols. Extra rows pull from scrollback via non-reflow
    // popline. CJK at col 3 of the 10-col line hits col 3 = new_cols-1.
    vt.set_size(4, 4);

    if(!assert_no_orphan_wide_chars(_test_failures, screen, vt.rows(), vt.cols()))
        return;

    // Find the scrollback-sourced row (starts with 'A') and verify
    for(int32_t r = 0; r < vt.rows(); r++) {
        ScreenCell cell{};
        (void)screen.get_cell({r, 0}, cell);
        if(cell.chars[0] == 'A') {
            ASSERT_SCREEN_CELL_CHAR(screen, r, 1, 'A');
            ASSERT_SCREEN_CELL_CHAR(screen, r, 2, 'A');
            // Col 3 should be blank (CJK cleared), not the CJK char
            ScreenCell col3{};
            (void)screen.get_cell({r, 3}, col3);
            ASSERT_TRUE(col3.chars[0] != 0x4E00);
            break;
        }
    }
}

// ============================================================================
// Group 3: Reflow non-fitting logical lines (#2, #3)
// ============================================================================

// #2: Shrink-reflow where a multi-row logical line doesn't fit. The bug
// discarded continuation rows, keeping only the first row. We verify all
// content is preserved across scrollback + screen.
TEST(regression_reflow_multirow_to_scrollback)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs_no_scrollrect);
    state.reset(true);
    Screen& screen = vt.screen();
    Scrollback& sb = vt.scrollback();
    sb.set_capacity(100);
    screen.enable_reflow(true);
    vt.set_size(4, 10);
    screen.reset(true);

    // Write a 30-char line that wraps to 3 rows on 10-col terminal.
    push(vt, "AAAAABBBBBC");
    push(vt, "CCCCDDDDDEE");
    push(vt, "EEEFFF\r\n");
    // One more line to put cursor on row 3
    push(vt, "ZZZZ");

    // Shrink to 2 rows — the 3-row logical line can't fit
    vt.set_size(2, 10);

    // Collect ALL text: scrollback + screen
    std::string all_text = collect_scrollback_text(sb);
    all_text += collect_screen_text(vt, screen);

    // The full content must be present — with the bug, only "AAAAA" survived
    ASSERT_TRUE(all_text.find("AAAAA") != std::string::npos);
    ASSERT_TRUE(all_text.find("BBBBB") != std::string::npos);
    ASSERT_TRUE(all_text.find("CCCCC") != std::string::npos);
    ASSERT_TRUE(all_text.find("DDDDD") != std::string::npos);
    ASSERT_TRUE(all_text.find("EEEEE") != std::string::npos);
    ASSERT_TRUE(all_text.find("FFF") != std::string::npos);
}

// #3: Cursor goes negative after resize. The big logical line falls off the
// top (setting new_cursor.row = 0), then the scroll-up adjustment subtracts
// further: new_cursor.row -= (new_row + 1). Without the clamp, this wraps
// to a negative value.
//
// Trigger: 4-row wrapped line + 1 blank row on a 5-row terminal. Cursor on
// the wrapped line. Shrink to 2 rows. The blank row fits, the wrapped line
// falls off top. No scrollback to fill remaining space, so scroll-up runs
// and cursor = 0 - 1 = -1.
TEST(regression_reflow_cursor_nonnegative)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs_no_scrollrect);
    state.reset(true);
    Screen& screen = vt.screen();
    // No scrollback — intentionally. Without scrollback to fill remaining rows,
    // the scroll-up adjustment at the end of resize_buffer fires.
    screen.enable_reflow(true);
    vt.set_size(5, 10);
    screen.reset(true);

    // Write 40-char wrapped line (fills rows 0-3 as one logical line)
    push(vt, "AAAAAAAAAA");  // row 0
    push(vt, "BBBBBBBBBB");  // row 1 (continuation)
    push(vt, "CCCCCCCCCC");  // row 2 (continuation)
    push(vt, "DDDDDDDDDD");  // row 3 (continuation)
    // Row 4 is blank. Cursor at deferred wrap position.

    // Move cursor into the big logical line
    push(vt, "\e[1;1H");
    ASSERT_CURSOR(state, 0, 0);

    // Shrink to 2 rows. The blank row at the bottom fits in new_row=1.
    // The 4-row wrapped line doesn't fit — falls off top.
    // Bug: cursor = 0 - (0 + 1) = -1. Fix: clamped to 0.
    vt.set_size(2, 10);

    Pos pos = state.cursor_pos();
    ASSERT_TRUE(pos.row >= 0);
    ASSERT_TRUE(pos.row < vt.rows());
    ASSERT_TRUE(pos.col >= 0);
    ASSERT_TRUE(pos.col < vt.cols());
}

// ============================================================================
// Group 4: Scroll region + resize (#9)
// ============================================================================

// #9: scrollregion_top exceeds new row count after shrink.
TEST(regression_scrollregion_shrink_below_top)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    push(vt, "\e[10;25r");
    vt.set_size(5, 80);

    Pos pos = state.cursor_pos();
    ASSERT_TRUE(pos.row >= 0);
    ASSERT_TRUE(pos.row < 5);

    callbacks_clear();

    push(vt, "\e[5;1H");
    push(vt, "\n");

    ASSERT_EQ(g_cb.scrollrect_count, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_row, 0);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_row, 5);

    pos = state.cursor_pos();
    ASSERT_TRUE(pos.row >= 0);
    ASSERT_TRUE(pos.row < 5);
}

// #9 variant: origin mode + degenerate scroll region.
TEST(regression_scrollregion_origin_mode_shrink)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    push(vt, "\e[?6h");
    push(vt, "\e[15;20r");
    vt.set_size(5, 80);

    Pos pos = state.cursor_pos();
    ASSERT_TRUE(pos.row >= 0);
    ASSERT_TRUE(pos.row < 5);
    ASSERT_TRUE(pos.col >= 0);

    callbacks_clear();

    push(vt, "\e[5;1H");
    push(vt, "\n");

    ASSERT_EQ(g_cb.scrollrect_count, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_row, 0);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_row, 5);

    pos = state.cursor_pos();
    ASSERT_TRUE(pos.row >= 0);
    ASSERT_TRUE(pos.row < 5);

    push(vt, "\e[?6l");
}

// ============================================================================
// Group 5: Scrollback capacity eviction during resize (#5)
// ============================================================================

// #5: enforce_capacity doesn't adjust sb_before_resize when evicting lines.
TEST(regression_scrollback_evict_resize_tracking)
{
    Scrollback::Impl impl;
    impl.capacity = 3;

    impl.push_line(make_row("OLD1", 10), false);
    impl.push_line(make_row("OLD2", 10), false);
    impl.push_line(make_row("OLD3", 10), false);
    ASSERT_EQ(impl.lines.size(), 3);

    impl.begin_resize();

    impl.push_line(make_row("NEW1", 10), false);
    impl.push_line(make_row("NEW2", 10), false);
    ASSERT_EQ(impl.lines.size(), 3);

    impl.commit_resize(5, 3, 10, 10);
    ASSERT_EQ(impl.push_track_count, 2);

    impl.begin_resize();
    impl.commit_resize(3, 5, 10, 10);

    ASSERT_EQ(impl.lines.size(), 1);

    ASSERT_EQ(impl.lines[0].cells[0].chars[0], 'O');
    ASSERT_EQ(impl.lines[0].cells[1].chars[0], 'L');
    ASSERT_EQ(impl.lines[0].cells[2].chars[0], 'D');
    ASSERT_EQ(impl.lines[0].cells[3].chars[0], '3');

    ASSERT_EQ(impl.push_track_count, 0);
    ASSERT_EQ(impl.push_track_start, 0);
}

// ============================================================================
// Group 6: UTF-8 decoder duplicate replacement char (#13)
// ============================================================================

// #13: UTF-8 decoder emits a duplicate U+FFFD when a multi-byte sequence is
// interrupted by an ASCII byte and the output buffer is exactly full after the
// first replacement char. The bug: bytes_remaining isn't reset before the early
// return, so the next decode call re-enters the incomplete-sequence path for
// the same byte.
TEST(regression_utf8_decoder_duplicate_replacement)
{
    auto ei = create_encoding(EncodingType::UTF8, 'u');
    ei->init();

    // Start a 2-byte sequence: \xC2 expects one continuation byte
    std::array<uint32_t, 1> cp{};
    auto r = ei->decode(cp, std::span{"\xC2", 1});
    ASSERT_EQ(r.codepoints_produced, 0);  // waiting for continuation

    // Send ASCII 'A' which interrupts the sequence. Output buffer has room
    // for 1 codepoint — the replacement char fills it, early return.
    r = ei->decode(cp, std::span{"A", 1});
    ASSERT_EQ(r.codepoints_produced, 1);
    ASSERT_EQ(cp[0], 0xFFFD);  // replacement for interrupted sequence
    ASSERT_EQ(r.bytes_consumed, 0);  // 'A' not consumed yet (buffer was full)

    // Decode again — should get 'A', not another replacement char.
    // Bug: bytes_remaining still nonzero → emits second U+FFFD.
    r = ei->decode(cp, std::span{"A", 1});
    ASSERT_EQ(r.codepoints_produced, 1);
    ASSERT_EQ(cp[0], static_cast<uint32_t>('A'));
}

// ============================================================================
// Group 7: Forward reflow wide char split (#14)
// ============================================================================

// #14: The forward reflow direction (old screen buffer → new buffer) splits
// double-width characters at row boundaries, same bug class as #10/#11 but in
// the main copy loop rather than the scrollback backfill path. Triggered when
// on-screen content (not scrollback) is reflowed to a narrower width.
//
// Setup: content stays on screen (no scrollback pushes). A CJK char at col 4
// of a 10-col row maps to col 4 (last column) in a 5-col reflow.
TEST(regression_wide_char_forward_reflow)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs_no_scrollrect);
    state.reset(true);
    Screen& screen = vt.screen();
    screen.enable_reflow(true);
    // No scrollback — content stays on screen through the forward path
    vt.set_size(3, 10);
    screen.reset(true);

    // CJK at col 4: "AAAA一BBBB" (10 cells, wide char at col 4-5)
    push(vt, "AAAA\xE4\xB8\x80""BBBB\r\n");
    // Two more rows so the terminal is full
    push(vt, "XXXX\r\n");
    push(vt, "YY");

    // All 3 rows are on screen. Resize to 5 cols — the forward reflow copies
    // old buffer content into the new buffer. CJK at col 4 = last column.
    vt.set_size(5, 5);

    if(!assert_no_orphan_wide_chars(_test_failures, screen, vt.rows(), vt.cols()))
        return;

    // Verify content survived
    std::string text = collect_screen_text(vt, screen);
    ASSERT_TRUE(text.find("AAAA") != std::string::npos);
    ASSERT_TRUE(text.find("BBBB") != std::string::npos);
    ASSERT_TRUE(text.find("XXXX") != std::string::npos);
}
