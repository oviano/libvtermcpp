// test_20_state_wrapping.cpp — state wrapping tests
// Ported from upstream libvterm t/20state_wrapping.test

#include "harness.h"

// 79th Column
TEST(state_wrapping_79th_column)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs_no_scrollrect);
    state.reset(true);
    callbacks_clear();

    // Move to column 75
    push(vt, "\e[75G");
    callbacks_clear();

    push(vt, "AAAAA");
    ASSERT_EQ(g_cb.putglyph_count, 5);
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 0x41);
    ASSERT_EQ(g_cb.putglyph[0].width, 1);
    ASSERT_EQ(g_cb.putglyph[0].row, 0);
    ASSERT_EQ(g_cb.putglyph[0].col, 74);
    ASSERT_EQ(g_cb.putglyph[1].chars[0], 0x41);
    ASSERT_EQ(g_cb.putglyph[1].width, 1);
    ASSERT_EQ(g_cb.putglyph[1].row, 0);
    ASSERT_EQ(g_cb.putglyph[1].col, 75);
    ASSERT_EQ(g_cb.putglyph[2].chars[0], 0x41);
    ASSERT_EQ(g_cb.putglyph[2].width, 1);
    ASSERT_EQ(g_cb.putglyph[2].row, 0);
    ASSERT_EQ(g_cb.putglyph[2].col, 76);
    ASSERT_EQ(g_cb.putglyph[3].chars[0], 0x41);
    ASSERT_EQ(g_cb.putglyph[3].width, 1);
    ASSERT_EQ(g_cb.putglyph[3].row, 0);
    ASSERT_EQ(g_cb.putglyph[3].col, 77);
    ASSERT_EQ(g_cb.putglyph[4].chars[0], 0x41);
    ASSERT_EQ(g_cb.putglyph[4].width, 1);
    ASSERT_EQ(g_cb.putglyph[4].row, 0);
    ASSERT_EQ(g_cb.putglyph[4].col, 78);
    ASSERT_CURSOR(state, 0, 79);
}

// 80th Column Phantom
TEST(state_wrapping_80th_column_phantom)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs_no_scrollrect);
    state.reset(true);
    callbacks_clear();

    // Move to column 75 and print 5 A's to reach column 79
    push(vt, "\e[75G");
    push(vt, "AAAAA");
    callbacks_clear();

    // Print one more A at column 79 - phantom position
    push(vt, "A");
    ASSERT_EQ(g_cb.putglyph_count, 1);
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 0x41);
    ASSERT_EQ(g_cb.putglyph[0].width, 1);
    ASSERT_EQ(g_cb.putglyph[0].row, 0);
    ASSERT_EQ(g_cb.putglyph[0].col, 79);
    ASSERT_CURSOR(state, 0, 79);
}

// Line Wraparound
TEST(state_wrapping_line_wraparound)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs_no_scrollrect);
    state.reset(true);
    callbacks_clear();

    // Move to column 75 and print 5 A's + 1 A at phantom position
    push(vt, "\e[75G");
    push(vt, "AAAAA");
    push(vt, "A");
    callbacks_clear();

    // Next character wraps to next line
    push(vt, "B");
    ASSERT_EQ(g_cb.putglyph_count, 1);
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 0x42);
    ASSERT_EQ(g_cb.putglyph[0].width, 1);
    ASSERT_EQ(g_cb.putglyph[0].row, 1);
    ASSERT_EQ(g_cb.putglyph[0].col, 0);
    ASSERT_CURSOR(state, 1, 1);
}

// Line Wraparound during combined write
TEST(state_wrapping_line_wraparound_combined)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs_no_scrollrect);
    state.reset(true);
    callbacks_clear();

    // Move to column 78 (1-indexed) on current row
    push(vt, "\e[78G");
    callbacks_clear();

    push(vt, "BBBCC");
    ASSERT_EQ(g_cb.putglyph_count, 5);
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 0x42);
    ASSERT_EQ(g_cb.putglyph[0].width, 1);
    ASSERT_EQ(g_cb.putglyph[0].row, 0);
    ASSERT_EQ(g_cb.putglyph[0].col, 77);
    ASSERT_EQ(g_cb.putglyph[1].chars[0], 0x42);
    ASSERT_EQ(g_cb.putglyph[1].width, 1);
    ASSERT_EQ(g_cb.putglyph[1].row, 0);
    ASSERT_EQ(g_cb.putglyph[1].col, 78);
    ASSERT_EQ(g_cb.putglyph[2].chars[0], 0x42);
    ASSERT_EQ(g_cb.putglyph[2].width, 1);
    ASSERT_EQ(g_cb.putglyph[2].row, 0);
    ASSERT_EQ(g_cb.putglyph[2].col, 79);
    ASSERT_EQ(g_cb.putglyph[3].chars[0], 0x43);
    ASSERT_EQ(g_cb.putglyph[3].width, 1);
    ASSERT_EQ(g_cb.putglyph[3].row, 1);
    ASSERT_EQ(g_cb.putglyph[3].col, 0);
    ASSERT_EQ(g_cb.putglyph[4].chars[0], 0x43);
    ASSERT_EQ(g_cb.putglyph[4].width, 1);
    ASSERT_EQ(g_cb.putglyph[4].row, 1);
    ASSERT_EQ(g_cb.putglyph[4].col, 1);
    ASSERT_CURSOR(state, 1, 2);
}

// DEC Auto Wrap Mode
TEST(state_wrapping_dec_auto_wrap_mode)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs_no_scrollrect);

    state.reset(true);
    callbacks_clear();

    // Disable auto wrap mode
    push(vt, "\e[?7l");
    callbacks_clear();

    // Move to column 75
    push(vt, "\e[75G");
    callbacks_clear();

    // Print 6 D's
    push(vt, "DDDDDD");
    ASSERT_EQ(g_cb.putglyph_count, 6);
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 0x44);
    ASSERT_EQ(g_cb.putglyph[0].width, 1);
    ASSERT_EQ(g_cb.putglyph[0].row, 0);
    ASSERT_EQ(g_cb.putglyph[0].col, 74);
    ASSERT_EQ(g_cb.putglyph[1].chars[0], 0x44);
    ASSERT_EQ(g_cb.putglyph[1].width, 1);
    ASSERT_EQ(g_cb.putglyph[1].row, 0);
    ASSERT_EQ(g_cb.putglyph[1].col, 75);
    ASSERT_EQ(g_cb.putglyph[2].chars[0], 0x44);
    ASSERT_EQ(g_cb.putglyph[2].width, 1);
    ASSERT_EQ(g_cb.putglyph[2].row, 0);
    ASSERT_EQ(g_cb.putglyph[2].col, 76);
    ASSERT_EQ(g_cb.putglyph[3].chars[0], 0x44);
    ASSERT_EQ(g_cb.putglyph[3].width, 1);
    ASSERT_EQ(g_cb.putglyph[3].row, 0);
    ASSERT_EQ(g_cb.putglyph[3].col, 77);
    ASSERT_EQ(g_cb.putglyph[4].chars[0], 0x44);
    ASSERT_EQ(g_cb.putglyph[4].width, 1);
    ASSERT_EQ(g_cb.putglyph[4].row, 0);
    ASSERT_EQ(g_cb.putglyph[4].col, 78);
    ASSERT_EQ(g_cb.putglyph[5].chars[0], 0x44);
    ASSERT_EQ(g_cb.putglyph[5].width, 1);
    ASSERT_EQ(g_cb.putglyph[5].row, 0);
    ASSERT_EQ(g_cb.putglyph[5].col, 79);
    ASSERT_CURSOR(state, 0, 79);

    callbacks_clear();

    // Another D overwrites at column 79 without wrapping
    push(vt, "D");
    ASSERT_EQ(g_cb.putglyph_count, 1);
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 0x44);
    ASSERT_EQ(g_cb.putglyph[0].width, 1);
    ASSERT_EQ(g_cb.putglyph[0].row, 0);
    ASSERT_EQ(g_cb.putglyph[0].col, 79);
    ASSERT_CURSOR(state, 0, 79);

    // Re-enable auto wrap mode
    push(vt, "\e[?7h");
}

// 80th column causes linefeed on wraparound
TEST(state_wrapping_80th_col_linefeed_on_wraparound)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs_no_scrollrect);

    state.reset(true);
    callbacks_clear();

    // Move to row 25, col 78 (1-indexed) and print ABC
    push(vt, "\e[25;78H");
    callbacks_clear();

    push(vt, "ABC");
    ASSERT_EQ(g_cb.putglyph_count, 3);
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 0x41);
    ASSERT_EQ(g_cb.putglyph[0].width, 1);
    ASSERT_EQ(g_cb.putglyph[0].row, 24);
    ASSERT_EQ(g_cb.putglyph[0].col, 77);
    ASSERT_EQ(g_cb.putglyph[1].chars[0], 0x42);
    ASSERT_EQ(g_cb.putglyph[1].width, 1);
    ASSERT_EQ(g_cb.putglyph[1].row, 24);
    ASSERT_EQ(g_cb.putglyph[1].col, 78);
    ASSERT_EQ(g_cb.putglyph[2].chars[0], 0x43);
    ASSERT_EQ(g_cb.putglyph[2].width, 1);
    ASSERT_EQ(g_cb.putglyph[2].row, 24);
    ASSERT_EQ(g_cb.putglyph[2].col, 79);
    ASSERT_CURSOR(state, 24, 79);

    callbacks_clear();

    // Printing D at phantom position triggers scroll and places D at start of last row
    push(vt, "D");
    ASSERT_EQ(g_cb.moverect_count, 1);
    ASSERT_EQ(g_cb.moverect[0].src.start_row, 1);
    ASSERT_EQ(g_cb.moverect[0].src.end_row, 25);
    ASSERT_EQ(g_cb.moverect[0].src.start_col, 0);
    ASSERT_EQ(g_cb.moverect[0].src.end_col, 80);
    ASSERT_EQ(g_cb.moverect[0].dest.start_row, 0);
    ASSERT_EQ(g_cb.moverect[0].dest.end_row, 24);
    ASSERT_EQ(g_cb.moverect[0].dest.start_col, 0);
    ASSERT_EQ(g_cb.moverect[0].dest.end_col, 80);
    // After scroll, D is placed at row 24, col 0
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 0x44);
    ASSERT_EQ(g_cb.putglyph[0].width, 1);
    ASSERT_EQ(g_cb.putglyph[0].row, 24);
    ASSERT_EQ(g_cb.putglyph[0].col, 0);
}

// 80th column phantom linefeed phantom cancelled by explicit cursor move
TEST(state_wrapping_phantom_cancelled_by_cursor_move)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs_no_scrollrect);

    state.reset(true);
    callbacks_clear();

    // Move to row 25, col 78 (1-indexed) and print ABC
    push(vt, "\e[25;78H");
    callbacks_clear();

    push(vt, "ABC");
    ASSERT_EQ(g_cb.putglyph_count, 3);
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 0x41);
    ASSERT_EQ(g_cb.putglyph[0].width, 1);
    ASSERT_EQ(g_cb.putglyph[0].row, 24);
    ASSERT_EQ(g_cb.putglyph[0].col, 77);
    ASSERT_EQ(g_cb.putglyph[1].chars[0], 0x42);
    ASSERT_EQ(g_cb.putglyph[1].width, 1);
    ASSERT_EQ(g_cb.putglyph[1].row, 24);
    ASSERT_EQ(g_cb.putglyph[1].col, 78);
    ASSERT_EQ(g_cb.putglyph[2].chars[0], 0x43);
    ASSERT_EQ(g_cb.putglyph[2].width, 1);
    ASSERT_EQ(g_cb.putglyph[2].row, 24);
    ASSERT_EQ(g_cb.putglyph[2].col, 79);
    ASSERT_CURSOR(state, 24, 79);

    callbacks_clear();

    // Explicit cursor move to row 25, col 1 cancels phantom; then D is placed there
    push(vt, "\e[25;1H" "D");
    ASSERT_EQ(g_cb.putglyph_count, 1);
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 0x44);
    ASSERT_EQ(g_cb.putglyph[0].width, 1);
    ASSERT_EQ(g_cb.putglyph[0].row, 24);
    ASSERT_EQ(g_cb.putglyph[0].col, 0);
}
