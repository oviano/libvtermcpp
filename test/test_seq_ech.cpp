// test_seq_ech.cpp -- per-sequence tests for ECH (CSI Ps X, Erase Character)
//
// ECH -- Erase Character (CSI Ps X)
//   Erases Ps characters starting at the cursor position, without moving the
//   cursor.  Default Ps is 1.  The erase extent is clamped to the right edge
//   of the screen.

#include "harness.h"

// ============================================================================
// ECH tests
// ============================================================================

// ECH with no parameter defaults to erasing 1 character
TEST(seq_ech_default_param)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Move cursor to row 5, col 20 (0-based) via 1-based CUP
    push(vt, "\e[6;21H");
    ASSERT_CURSOR(state, 5, 20);

    callbacks_clear();

    // ECH with no param -- erase 1 character
    push(vt, "\e[X");

    ASSERT_EQ(g_cb.erase_count, 1);
    ASSERT_EQ(g_cb.erase[0].rect.start_row, 5);
    ASSERT_EQ(g_cb.erase[0].rect.end_row,   6);
    ASSERT_EQ(g_cb.erase[0].rect.start_col, 20);
    ASSERT_EQ(g_cb.erase[0].rect.end_col,   21);
}

// ECH with explicit count erases that many characters
TEST(seq_ech_explicit_count)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Move cursor to row 5, col 20 (0-based)
    push(vt, "\e[6;21H");
    ASSERT_CURSOR(state, 5, 20);

    callbacks_clear();

    // ECH 10 -- erase 10 characters
    push(vt, "\e[10X");

    ASSERT_EQ(g_cb.erase_count, 1);
    ASSERT_EQ(g_cb.erase[0].rect.start_row, 5);
    ASSERT_EQ(g_cb.erase[0].rect.end_row,   6);
    ASSERT_EQ(g_cb.erase[0].rect.start_col, 20);
    ASSERT_EQ(g_cb.erase[0].rect.end_col,   30);
}

// ECH count that exceeds the right edge is clamped to screen width
TEST(seq_ech_clamp_at_right_edge)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Move cursor to row 5, col 75 (0-based)
    push(vt, "\e[6;76H");
    ASSERT_CURSOR(state, 5, 75);

    callbacks_clear();

    // ECH 999 -- should clamp end_col to 80 (screen width)
    push(vt, "\e[999X");

    ASSERT_EQ(g_cb.erase_count, 1);
    ASSERT_EQ(g_cb.erase[0].rect.start_row, 5);
    ASSERT_EQ(g_cb.erase[0].rect.end_row,   6);
    ASSERT_EQ(g_cb.erase[0].rect.start_col, 75);
    ASSERT_EQ(g_cb.erase[0].rect.end_col,   80);
}

// ECH does not move the cursor
TEST(seq_ech_does_not_move_cursor)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Move cursor to row 5, col 20 (0-based)
    push(vt, "\e[6;21H");
    ASSERT_CURSOR(state, 5, 20);

    // ECH 10 -- cursor must not move
    push(vt, "\e[10X");
    ASSERT_CURSOR(state, 5, 20);
}

// ECH starting at column 0
TEST(seq_ech_at_col_0)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Move cursor to row 5, col 0 (0-based)
    push(vt, "\e[6;1H");
    ASSERT_CURSOR(state, 5, 0);

    callbacks_clear();

    // ECH 5 -- erase 5 characters from col 0
    push(vt, "\e[5X");

    ASSERT_EQ(g_cb.erase_count, 1);
    ASSERT_EQ(g_cb.erase[0].rect.start_row, 5);
    ASSERT_EQ(g_cb.erase[0].rect.end_row,   6);
    ASSERT_EQ(g_cb.erase[0].rect.start_col, 0);
    ASSERT_EQ(g_cb.erase[0].rect.end_col,   5);
}
