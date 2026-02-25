// test_seq_dsr.cpp -- per-sequence tests for DSR (CSI n) and DECDSR (CSI ? n)
//
// DSR (Device Status Report):
//   CSI 5 n  -- operating status report -> responds CSI 0 n (OK)
//   CSI 6 n  -- cursor position report (CPR) -> responds CSI Pr ; Pc R (1-based)
//
// DECDSR (DEC-private DSR):
//   CSI ? 6 n -- extended CPR -> responds CSI ? Pr ; Pc R (1-based)

#include "harness.h"

// ============================================================================
// DSR operating status -- CSI 5 n
// ============================================================================

// CSI 5 n should respond with CSI 0 n indicating "ready, no malfunctions"
TEST(seq_dsr_operating_status)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    output_init(vt);
    state.reset(true);
    callbacks_clear();
    output_clear();

    push(vt, "\e[5n");
    ASSERT_OUTPUT_BYTES("\e[0n", 4);
}

// ============================================================================
// DSR cursor position report at origin
// ============================================================================

// CSI 6 n at the origin (row 0, col 0) should respond CSI 1;1 R
TEST(seq_dsr_cpr_at_origin)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    output_init(vt);
    state.reset(true);
    callbacks_clear();
    output_clear();

    push(vt, "\e[6n");
    ASSERT_OUTPUT_BYTES("\e[1;1R", 6);
}

// ============================================================================
// DSR cursor position report at a specific position
// ============================================================================

// Move cursor to row 10, col 20 (1-based via CUP), then CSI 6 n.
// Response should be CSI 10;20 R.
TEST(seq_dsr_cpr_at_position)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    output_init(vt);
    state.reset(true);
    callbacks_clear();
    output_clear();

    // Move cursor to row 10, col 20 (1-based) => row 9, col 19 (0-based)
    push(vt, "\e[10;20H");
    ASSERT_CURSOR(state, 9, 19);

    output_clear();
    push(vt, "\e[6n");
    ASSERT_OUTPUT_BYTES("\e[10;20R", 8);
}

// ============================================================================
// DECDSR -- CSI ? 6 n
// ============================================================================

// Move cursor to row 10, col 10 (1-based), then CSI ? 6 n.
// Response should be CSI ? 10;10 R (physical position).
TEST(seq_dsr_deccpr)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    output_init(vt);
    state.reset(true);
    callbacks_clear();
    output_clear();

    // Move cursor to row 10, col 10 (1-based) => row 9, col 9 (0-based)
    push(vt, "\e[10;10H");
    ASSERT_CURSOR(state, 9, 9);

    output_clear();
    push(vt, "\e[?6n");
    ASSERT_OUTPUT_BYTES("\e[?10;10R", 9);
}

// ============================================================================
// DSR operating status with DEC-private prefix
// ============================================================================

// CSI ? 5 n should also respond with CSI ? 0 n (operating status OK)
TEST(seq_dsr_dec_operating_status)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    output_init(vt);
    state.reset(true);
    callbacks_clear();
    output_clear();

    push(vt, "\e[?5n");
    ASSERT_OUTPUT_BYTES("\e[?0n", 5);
}
