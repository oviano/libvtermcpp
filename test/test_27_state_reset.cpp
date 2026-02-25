// test_27_state_reset.cpp — state reset tests
// Ported from upstream libvterm t/27state_reset.test

#include "harness.h"

// RIS homes cursor
TEST(state_reset_ris_homes_cursor)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);

    state.reset(true);
    callbacks_clear();

    // Move cursor to row 4, col 4
    push(vt, "\e[5;5H");
    ASSERT_CURSOR(state, 4, 4);

    // RIS: ESC c
    callbacks_clear();
    push(vt, "\ec");
    ASSERT_CURSOR(state, 0, 0);
}

// RIS cancels scrolling region
TEST(state_reset_ris_cancels_scrolling_region)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);

    state.reset(true);
    callbacks_clear();

    // Set scrolling region to rows 5..10: CSI 5;10 r
    push(vt, "\e[5;10r");

    // RIS then move to last line and newline
    callbacks_clear();
    push(vt, "\ec\e[25H\n");

    // scrollrect 0..25,0..80 => +1,+0
    ASSERT_EQ(g_cb.scrollrect_count, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_row, 0);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_row, 25);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_col, 0);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_col, 80);
    ASSERT_EQ(g_cb.scrollrect[0].downward, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rightward, 0);
}

// RIS erases screen
TEST(state_reset_ris_erases_screen)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);

    state.reset(true);
    callbacks_clear();

    push(vt, "ABCDE");

    // RIS: ESC c
    callbacks_clear();
    push(vt, "\ec");

    // erase 0..25,0..80
    ASSERT_EQ(g_cb.erase_count, 1);
    ASSERT_EQ(g_cb.erase[0].rect.start_row, 0);
    ASSERT_EQ(g_cb.erase[0].rect.end_row, 25);
    ASSERT_EQ(g_cb.erase[0].rect.start_col, 0);
    ASSERT_EQ(g_cb.erase[0].rect.end_col, 80);
}

// RIS clears tabstops
TEST(state_reset_ris_clears_tabstops)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);

    state.reset(true);
    callbacks_clear();

    // Move to column 5, set a tab stop, go back to column 1, then tab
    push(vt, "\e[5G\eH\e[G\t");
    ASSERT_CURSOR(state, 0, 4);

    // RIS then tab - should go to default tab stop at column 8
    push(vt, "\ec\t");
    ASSERT_CURSOR(state, 0, 8);
}
