// test_seq_cup_hvp.cpp — per-sequence tests for CUP (CSI H) and HVP (CSI f)
//
// CUP: Cursor Position           — CSI Pr ; Pc H
// HVP: Horizontal and Vertical   — CSI Pr ; Pc f
//
// CSI parameters are 1-based; ASSERT_CURSOR values are 0-based.

#include "harness.h"

// CUP with no params \e[H goes to 0,0
TEST(seq_cup_default_params)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Move cursor somewhere away from home
    push(vt, "\x1b[5;20H");
    ASSERT_CURSOR(state, 4, 19);

    // CUP with no parameters should go to 0,0
    push(vt, "\x1b[H");
    ASSERT_CURSOR(state, 0, 0);
}

// CUP with row only \e[10H goes to row 9, col 0
TEST(seq_cup_row_only)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // CUP with row 10 only — column defaults to 1 (0-based: col 0)
    push(vt, "\x1b[10H");
    ASSERT_CURSOR(state, 9, 0);
}

// CUP \e[10;20H goes to row 9, col 19
TEST(seq_cup_explicit_position)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    push(vt, "\x1b[10;20H");
    ASSERT_CURSOR(state, 9, 19);
}

// CUP \e[999;999H clamped to bottom-right corner (row 24, col 79)
TEST(seq_cup_clamp_beyond_screen)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    push(vt, "\x1b[999;999H");
    ASSERT_CURSOR(state, 24, 79);
}

// With DECOM on and DECSTBM 5;20, CUP \e[1;1H goes to row 4, col 0
TEST(seq_cup_with_origin_mode)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Set scroll region rows 5..20 (1-based)
    push(vt, "\x1b[5;20r");

    // Enable DEC origin mode
    push(vt, "\x1b[?6h");

        // CUP \e[1;1H — in origin mode, row 1 maps to top of scroll region
    // (row 5 in 1-based = row 4 in 0-based), col 1 maps to col 0
    push(vt, "\x1b[1;1H");
    ASSERT_CURSOR(state, 4, 0);

    // Disable origin mode to leave clean state
    push(vt, "\x1b[?6l");
}

// Print at col 79 to enter phantom state, then CUP clears it
TEST(seq_cup_cancels_phantom)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

        // Move to the last column and print a character to enter phantom state.
    // After writing 80 chars on a row the cursor stays at col 79 but is in
    // the phantom (pending-wrap) state.
    push(vt, "\x1b[1;80H");
    ASSERT_CURSOR(state, 0, 79);

    push(vt, "X");
    // Cursor remains at col 79 in phantom state
    ASSERT_CURSOR(state, 0, 79);

    // CUP should cancel phantom state and move cursor
    push(vt, "\x1b[5;5H");
    ASSERT_CURSOR(state, 4, 4);

        // Printing a character after CUP should NOT cause a wrap/newline —
    // it should simply place the character at the current position
    push(vt, "A");
    ASSERT_CURSOR(state, 4, 5);
}

// HVP \e[10;20f behaves identically to CUP
TEST(seq_hvp_matches_cup)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // HVP with no params — goes to 0,0
    push(vt, "\x1b[5;20H");
    ASSERT_CURSOR(state, 4, 19);
    push(vt, "\x1b[f");
    ASSERT_CURSOR(state, 0, 0);

    // HVP with explicit position
    push(vt, "\x1b[10;20f");
    ASSERT_CURSOR(state, 9, 19);

    // HVP clamped beyond screen
    push(vt, "\x1b[999;999f");
    ASSERT_CURSOR(state, 24, 79);
}

// HVP with DECOM behaves same as CUP
TEST(seq_hvp_with_origin_mode)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Set scroll region rows 5..20 (1-based)
    push(vt, "\x1b[5;20r");

    // Enable DEC origin mode
    push(vt, "\x1b[?6h");

        // HVP \e[1;1f — same as CUP: origin mode makes row 1 = top of
    // scroll region (row 4 in 0-based), col 1 = col 0
    push(vt, "\x1b[1;1f");
    ASSERT_CURSOR(state, 4, 0);

    // HVP to row 3, col 10 within scroll region
    push(vt, "\x1b[3;10f");
    // Row 3 within region starting at row 5 (1-based) = row index 4+2 = 6
    ASSERT_CURSOR(state, 6, 9);

    // Disable origin mode to leave clean state
    push(vt, "\x1b[?6l");
}
