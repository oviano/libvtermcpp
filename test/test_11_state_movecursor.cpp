// test_11_state_movecursor.cpp — state layer cursor movement tests
// Ported from upstream libvterm t/11state_movecursor.test

#include "harness.h"

// Implicit
TEST(state_movecursor_implicit)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    push(vt, "ABC");
    ASSERT_CURSOR(state, 0, 3);
}

// Backspace
TEST(state_movecursor_backspace)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    push(vt, "ABC");
    push(vt, "\b");
    ASSERT_CURSOR(state, 0, 2);
}

// Horizontal Tab
TEST(state_movecursor_horizontal_tab)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    push(vt, "ABC");
    push(vt, "\t");
    ASSERT_CURSOR(state, 0, 8);
}

// Carriage Return
TEST(state_movecursor_carriage_return)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    push(vt, "ABC");
    push(vt, "\r");
    ASSERT_CURSOR(state, 0, 0);
}

// Linefeed
TEST(state_movecursor_linefeed)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Upstream state: after CR cursor is at 0,0; then LF moves to 1,0
    push(vt, "ABC");
    push(vt, "\r");
    push(vt, "\n");
    ASSERT_CURSOR(state, 1, 0);
}

// Backspace bounded by lefthand edge
TEST(state_movecursor_backspace_bounded_left)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    push(vt, "\e[4;2H");
    ASSERT_CURSOR(state, 3, 1);
    push(vt, "\b");
    ASSERT_CURSOR(state, 3, 0);
    push(vt, "\b");
    ASSERT_CURSOR(state, 3, 0);
}

// Backspace cancels phantom
TEST(state_movecursor_backspace_cancels_phantom)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    push(vt, "\e[4;80H");
    ASSERT_CURSOR(state, 3, 79);
    push(vt, "X");
    ASSERT_CURSOR(state, 3, 79);
    push(vt, "\b");
    ASSERT_CURSOR(state, 3, 78);
}

// HT bounded by righthand edge
TEST(state_movecursor_ht_bounded_right)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    push(vt, "\e[1;78H");
    ASSERT_CURSOR(state, 0, 77);
    push(vt, "\t");
    ASSERT_CURSOR(state, 0, 79);
    push(vt, "\t");
    ASSERT_CURSOR(state, 0, 79);
}

// Index
TEST(state_movecursor_index)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    push(vt, "ABC\eD");
    ASSERT_CURSOR(state, 1, 3);
}

// Reverse Index
TEST(state_movecursor_reverse_index)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    push(vt, "ABC\eD");
    push(vt, "\eM");
    ASSERT_CURSOR(state, 0, 3);
}

// Newline
TEST(state_movecursor_newline)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    push(vt, "ABC\eD");
    push(vt, "\eM");
    push(vt, "\eE");
    ASSERT_CURSOR(state, 1, 0);
}

// Cursor Forward
TEST(state_movecursor_cursor_forward)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    push(vt, "\e[B");
    ASSERT_CURSOR(state, 1, 0);
    push(vt, "\e[3B");
    ASSERT_CURSOR(state, 4, 0);
    push(vt, "\e[0B");
    ASSERT_CURSOR(state, 5, 0);
}

// Cursor Down
TEST(state_movecursor_cursor_down)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Move to row 5 first
    push(vt, "\e[B");
    push(vt, "\e[3B");
    push(vt, "\e[0B");
    ASSERT_CURSOR(state, 5, 0);

    push(vt, "\e[C");
    ASSERT_CURSOR(state, 5, 1);
    push(vt, "\e[3C");
    ASSERT_CURSOR(state, 5, 4);
    push(vt, "\e[0C");
    ASSERT_CURSOR(state, 5, 5);
}

// Cursor Up
TEST(state_movecursor_cursor_up)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Move to row 5, col 5 first
    push(vt, "\e[B");
    push(vt, "\e[3B");
    push(vt, "\e[0B");
    push(vt, "\e[C");
    push(vt, "\e[3C");
    push(vt, "\e[0C");
    ASSERT_CURSOR(state, 5, 5);

    push(vt, "\e[A");
    ASSERT_CURSOR(state, 4, 5);
    push(vt, "\e[3A");
    ASSERT_CURSOR(state, 1, 5);
    push(vt, "\e[0A");
    ASSERT_CURSOR(state, 0, 5);
}

// Cursor Backward
TEST(state_movecursor_cursor_backward)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Move to row 0, col 5 first
    push(vt, "\e[B");
    push(vt, "\e[3B");
    push(vt, "\e[0B");
    push(vt, "\e[C");
    push(vt, "\e[3C");
    push(vt, "\e[0C");
    push(vt, "\e[A");
    push(vt, "\e[3A");
    push(vt, "\e[0A");
    ASSERT_CURSOR(state, 0, 5);

    push(vt, "\e[D");
    ASSERT_CURSOR(state, 0, 4);
    push(vt, "\e[3D");
    ASSERT_CURSOR(state, 0, 1);
    push(vt, "\e[0D");
    ASSERT_CURSOR(state, 0, 0);
}

// Cursor Next Line
TEST(state_movecursor_cursor_next_line)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Continue from cursor at 0,0
    push(vt, "   ");
    ASSERT_CURSOR(state, 0, 3);
    push(vt, "\e[E");
    ASSERT_CURSOR(state, 1, 0);
    push(vt, "   ");
    ASSERT_CURSOR(state, 1, 3);
    push(vt, "\e[2E");
    ASSERT_CURSOR(state, 3, 0);
    push(vt, "\e[0E");
    ASSERT_CURSOR(state, 4, 0);
}

// Cursor Previous Line
TEST(state_movecursor_cursor_prev_line)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Set up cursor at row 4, col 0 (matching upstream state)
    push(vt, "   ");
    push(vt, "\e[E");
    push(vt, "   ");
    push(vt, "\e[2E");
    push(vt, "\e[0E");
    ASSERT_CURSOR(state, 4, 0);

    push(vt, "   ");
    ASSERT_CURSOR(state, 4, 3);
    push(vt, "\e[F");
    ASSERT_CURSOR(state, 3, 0);
    push(vt, "   ");
    ASSERT_CURSOR(state, 3, 3);
    push(vt, "\e[2F");
    ASSERT_CURSOR(state, 1, 0);
    push(vt, "\e[0F");
    ASSERT_CURSOR(state, 0, 0);
}

// Cursor Horizontal Absolute
TEST(state_movecursor_cursor_horizontal_absolute)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    push(vt, "\n");
    ASSERT_CURSOR(state, 1, 0);
    push(vt, "\e[20G");
    ASSERT_CURSOR(state, 1, 19);
    push(vt, "\e[G");
    ASSERT_CURSOR(state, 1, 0);
}

// Cursor Position
TEST(state_movecursor_cursor_position)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    push(vt, "\e[10;5H");
    ASSERT_CURSOR(state, 9, 4);
    push(vt, "\e[8H");
    ASSERT_CURSOR(state, 7, 0);
    push(vt, "\e[H");
    ASSERT_CURSOR(state, 0, 0);
}

// Cursor Position cancels phantom
TEST(state_movecursor_cursor_position_cancels_phantom)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    push(vt, "\e[10;78H");
    ASSERT_CURSOR(state, 9, 77);
    push(vt, "ABC");
    ASSERT_CURSOR(state, 9, 79);
    push(vt, "\e[10;80H");
    push(vt, "C");
    ASSERT_CURSOR(state, 9, 79);
    push(vt, "X");
    ASSERT_CURSOR(state, 10, 1);
}

// Bounds Checking
TEST(state_movecursor_bounds_checking)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    push(vt, "\e[A");
    ASSERT_CURSOR(state, 0, 0);
    push(vt, "\e[D");
    ASSERT_CURSOR(state, 0, 0);
    push(vt, "\e[25;80H");
    ASSERT_CURSOR(state, 24, 79);
    push(vt, "\e[B");
    ASSERT_CURSOR(state, 24, 79);
    push(vt, "\e[C");
    ASSERT_CURSOR(state, 24, 79);
    push(vt, "\e[E");
    ASSERT_CURSOR(state, 24, 0);
    push(vt, "\e[H");
    ASSERT_CURSOR(state, 0, 0);
    push(vt, "\e[F");
    ASSERT_CURSOR(state, 0, 0);
    push(vt, "\e[999G");
    ASSERT_CURSOR(state, 0, 79);
    push(vt, "\e[99;99H");
    ASSERT_CURSOR(state, 24, 79);
}

// Horizontal Position Absolute
TEST(state_movecursor_horizontal_position_absolute)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    push(vt, "\e[5`");
    ASSERT_CURSOR(state, 0, 4);
}

// Horizontal Position Relative
TEST(state_movecursor_horizontal_position_relative)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    push(vt, "\e[5`");
    push(vt, "\e[3a");
    ASSERT_CURSOR(state, 0, 7);
}

// Horizontal Position Backward
TEST(state_movecursor_horizontal_position_backward)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    push(vt, "\e[5`");
    push(vt, "\e[3a");
    push(vt, "\e[3j");
    ASSERT_CURSOR(state, 0, 4);
}

// Horizontal and Vertical Position
TEST(state_movecursor_horizontal_and_vertical_position)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    push(vt, "\e[3;3f");
    ASSERT_CURSOR(state, 2, 2);
}

// Vertical Position Absolute
TEST(state_movecursor_vertical_position_absolute)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    push(vt, "\e[3;3f");
    push(vt, "\e[5d");
    ASSERT_CURSOR(state, 4, 2);
}

// Vertical Position Relative
TEST(state_movecursor_vertical_position_relative)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    push(vt, "\e[3;3f");
    push(vt, "\e[5d");
    push(vt, "\e[2e");
    ASSERT_CURSOR(state, 6, 2);
}

// Vertical Position Backward
TEST(state_movecursor_vertical_position_backward)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    push(vt, "\e[3;3f");
    push(vt, "\e[5d");
    push(vt, "\e[2e");
    push(vt, "\e[2k");
    ASSERT_CURSOR(state, 4, 2);
}

// Horizontal Tab
TEST(state_movecursor_horizontal_tab_detailed)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    push(vt, "\t");
    ASSERT_CURSOR(state, 0, 8);
    push(vt, "   ");
    ASSERT_CURSOR(state, 0, 11);
    push(vt, "\t");
    ASSERT_CURSOR(state, 0, 16);
    push(vt, "       ");
    ASSERT_CURSOR(state, 0, 23);
    push(vt, "\t");
    ASSERT_CURSOR(state, 0, 24);
    push(vt, "        ");
    ASSERT_CURSOR(state, 0, 32);
    push(vt, "\t");
    ASSERT_CURSOR(state, 0, 40);
}

// Cursor Horizontal Tab
TEST(state_movecursor_cursor_horizontal_tab)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Set up cursor at col 40 (matching upstream state after HT tests)
    push(vt, "\t");
    push(vt, "   ");
    push(vt, "\t");
    push(vt, "       ");
    push(vt, "\t");
    push(vt, "        ");
    push(vt, "\t");
    ASSERT_CURSOR(state, 0, 40);

    push(vt, "\e[I");
    ASSERT_CURSOR(state, 0, 48);
    push(vt, "\e[2I");
    ASSERT_CURSOR(state, 0, 64);
}

// Cursor Backward Tab
TEST(state_movecursor_cursor_backward_tab)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Set up cursor at col 64 (matching upstream state after CHT tests)
    push(vt, "\t");
    push(vt, "   ");
    push(vt, "\t");
    push(vt, "       ");
    push(vt, "\t");
    push(vt, "        ");
    push(vt, "\t");
    push(vt, "\e[I");
    push(vt, "\e[2I");
    ASSERT_CURSOR(state, 0, 64);

    push(vt, "\e[Z");
    ASSERT_CURSOR(state, 0, 56);
    push(vt, "\e[2Z");
    ASSERT_CURSOR(state, 0, 40);
}
