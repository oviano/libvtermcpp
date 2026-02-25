// test_seq_decawm.cpp -- tests for Mode 7 (DECAWM / Auto Wrap Mode)
//
// DECAWM controls whether the cursor wraps to the next line when a
// character is printed past the right margin. When enabled (default),
// printing at the last column enters a "phantom" state; the next
// character causes a wrap. When disabled, printing at the last column
// keeps overwriting that position.

#include "harness.h"

// ===== DECAWM enabled: cursor wraps to next row after last column =====
TEST(seq_decawm_enabled_wraps)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Ensure autowrap is enabled (default): CSI ? 7 h
    push(vt, "\e[?7h");

    // Move to column 79 (1-based 80) on row 1
    push(vt, "\e[1;80H");
    ASSERT_CURSOR(state, 0, 79);

    callbacks_clear();

    // Print 'A' at col 79 -- enters phantom state, cursor reports col 79
    push(vt, "A");
    ASSERT_EQ(g_cb.putglyph_count, 1);
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 'A');
    ASSERT_EQ(g_cb.putglyph[0].row, 0);
    ASSERT_EQ(g_cb.putglyph[0].col, 79);
    ASSERT_CURSOR(state, 0, 79);

    callbacks_clear();

    // Print 'B' -- should wrap to next row
    push(vt, "B");
    ASSERT_EQ(g_cb.putglyph_count, 1);
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 'B');
    ASSERT_EQ(g_cb.putglyph[0].row, 1);
    ASSERT_EQ(g_cb.putglyph[0].col, 0);
    ASSERT_CURSOR(state, 1, 1);
}

// ===== DECAWM disabled: cursor stays at last column =====
TEST(seq_decawm_disabled_no_wrap)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Disable autowrap: CSI ? 7 l
    push(vt, "\e[?7l");

    // Move to column 79
    push(vt, "\e[1;80H");
    ASSERT_CURSOR(state, 0, 79);

    callbacks_clear();

    // Print multiple characters -- all should overwrite at col 79
    push(vt, "ABC");
    ASSERT_EQ(g_cb.putglyph_count, 3);
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 'A');
    ASSERT_EQ(g_cb.putglyph[0].row, 0);
    ASSERT_EQ(g_cb.putglyph[0].col, 79);
    ASSERT_EQ(g_cb.putglyph[1].chars[0], 'B');
    ASSERT_EQ(g_cb.putglyph[1].row, 0);
    ASSERT_EQ(g_cb.putglyph[1].col, 79);
    ASSERT_EQ(g_cb.putglyph[2].chars[0], 'C');
    ASSERT_EQ(g_cb.putglyph[2].row, 0);
    ASSERT_EQ(g_cb.putglyph[2].col, 79);
    // Cursor stays at col 79
    ASSERT_CURSOR(state, 0, 79);
}

// ===== Phantom state: printing at last col reports cursor at col 79 =====
TEST(seq_decawm_phantom_state)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Autowrap is on by default. Move to col 79 and print a char.
    push(vt, "\e[1;80H");
    callbacks_clear();

    push(vt, "X");
    // 'X' placed at col 79, cursor remains at (0,79) in phantom state
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 'X');
    ASSERT_EQ(g_cb.putglyph[0].col, 79);
    ASSERT_CURSOR(state, 0, 79);
}

// ===== Phantom resolved by printing another character =====
TEST(seq_decawm_phantom_resolved_by_print)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Move to col 79, print char to enter phantom
    push(vt, "\e[1;80H");
    push(vt, "X");
    ASSERT_CURSOR(state, 0, 79);

    callbacks_clear();

    // Print another char -- phantom resolves, wraps to (1,0)
    push(vt, "Y");
    ASSERT_EQ(g_cb.putglyph_count, 1);
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 'Y');
    ASSERT_EQ(g_cb.putglyph[0].row, 1);
    ASSERT_EQ(g_cb.putglyph[0].col, 0);
    ASSERT_CURSOR(state, 1, 1);
}

// ===== Phantom cancelled by CUP =====
TEST(seq_decawm_phantom_cancelled_by_cup)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Move to col 79, print char to enter phantom
    push(vt, "\e[1;80H");
    push(vt, "X");
    ASSERT_CURSOR(state, 0, 79);

    // CUP cancels phantom and moves cursor
    push(vt, "\e[5;10H");
    ASSERT_CURSOR(state, 4, 9);

    callbacks_clear();

    // Print 'Z' at new position -- should NOT wrap, should be at (4,9)
    push(vt, "Z");
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 'Z');
    ASSERT_EQ(g_cb.putglyph[0].row, 4);
    ASSERT_EQ(g_cb.putglyph[0].col, 9);
    ASSERT_CURSOR(state, 4, 10);
}

// ===== Autowrap at bottom row causes scroll =====
TEST(seq_decawm_scroll_at_bottom)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Move to last row, last col: row 25, col 80 (1-based)
    push(vt, "\e[25;80H");
    ASSERT_CURSOR(state, 24, 79);

    // Print 'A' at phantom position
    push(vt, "A");
    ASSERT_CURSOR(state, 24, 79);

    callbacks_clear();

    // Print 'B' -- should trigger scroll (scrollrect downward=1) then print at (24,0)
    push(vt, "B");
    ASSERT_EQ(g_cb.scrollrect_count, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_row, 0);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_row, 25);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_col, 0);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_col, 80);
    ASSERT_EQ(g_cb.scrollrect[0].downward, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rightward, 0);

    // 'B' placed at bottom-left after scroll
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 'B');
    ASSERT_EQ(g_cb.putglyph[0].row, 24);
    ASSERT_EQ(g_cb.putglyph[0].col, 0);
}
