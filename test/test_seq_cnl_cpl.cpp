// test_seq_cnl_cpl.cpp — per-sequence tests for CNL (CSI E) and CPL (CSI F)
//
// CNL (Cursor Next Line):     ESC [ <n> E — move down N lines, reset col to 0
// CPL (Cursor Previous Line): ESC [ <n> F — move up N lines, reset col to 0

#include "harness.h"

// CNL with default parameter: \e[E moves down 1 row and resets column to 0
TEST(seq_cnl_default_param)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Position cursor at row 9, col 39 (1-based: 10;40)
    push(vt, "\e[10;40H");
    ASSERT_CURSOR(state, 9, 39);

    // CNL with no parameter — should move down 1, col to 0
    push(vt, "\e[E");
    ASSERT_CURSOR(state, 10, 0);
}

// CNL with explicit count: \e[3E moves down 3 rows and resets column to 0
TEST(seq_cnl_explicit_count)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Position cursor at row 9, col 39 (1-based: 10;40)
    push(vt, "\e[10;40H");
    ASSERT_CURSOR(state, 9, 39);

    // CNL with count 3 — should move down 3, col to 0
    push(vt, "\e[3E");
    ASSERT_CURSOR(state, 12, 0);
}

// CNL with zero parameter: \e[0E is treated as 1 (moves down 1)
TEST(seq_cnl_zero_param)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Position cursor at row 9, col 39 (1-based: 10;40)
    push(vt, "\e[10;40H");
    ASSERT_CURSOR(state, 9, 39);

    // CNL with 0 parameter — treated as 1, move down 1, col to 0
    push(vt, "\e[0E");
    ASSERT_CURSOR(state, 10, 0);
}

// CNL at bottom row: cursor row stays clamped, column still goes to 0
TEST(seq_cnl_clamp_bottom)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Position cursor at bottom-right: row 24, col 39 (1-based: 25;40)
    push(vt, "\e[25;40H");
    ASSERT_CURSOR(state, 24, 39);

    // CNL at the bottom row — row stays at 24, col goes to 0
    push(vt, "\e[E");
    ASSERT_CURSOR(state, 24, 0);
}

// CPL with default parameter: \e[F moves up 1 row and resets column to 0
TEST(seq_cpl_default_param)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Position cursor at row 9, col 39 (1-based: 10;40)
    push(vt, "\e[10;40H");
    ASSERT_CURSOR(state, 9, 39);

    // CPL with no parameter — should move up 1, col to 0
    push(vt, "\e[F");
    ASSERT_CURSOR(state, 8, 0);
}

// CPL at top row: cursor row stays clamped at 0, column still goes to 0
TEST(seq_cpl_clamp_top)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Position cursor at top row, col 39 (1-based: 1;40)
    push(vt, "\e[1;40H");
    ASSERT_CURSOR(state, 0, 39);

    // CPL at the top row — row stays at 0, col goes to 0
    push(vt, "\e[F");
    ASSERT_CURSOR(state, 0, 0);
}
