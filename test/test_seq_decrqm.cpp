// test_seq_decrqm.cpp -- per-sequence tests for DECRQM (CSI ? Ps $ p)
//
// DECRQM (Request Mode) queries the current state of a DEC private mode.
// Response format: CSI ? Ps ; Pm $ y
//   Pm values: 0 = not recognized, 1 = set, 2 = reset,
//              3 = permanently set, 4 = permanently reset
//
// The query is sent as: ESC [ ? <mode> $ p
// The response is:      ESC [ ? <mode> ; <value> $ y

#include "harness.h"

// ============================================================================
// DECRQM: mode set -- DECAWM (mode 7) enabled
// ============================================================================

// Enable DECAWM (mode 7), then query it. Should respond with value 1 (set).
TEST(seq_decrqm_mode_set)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    output_init(vt);
    state.reset(true);
    callbacks_clear();
    output_clear();

    // Enable DECAWM
    push(vt, "\e[?7h");

    output_clear();
    push(vt, "\e[?7$p");
    // Response: ESC [ ? 7 ; 1 $ y  -- 9 bytes
    ASSERT_OUTPUT_BYTES("\e[?7;1$y", 8);
}

// ============================================================================
// DECRQM: mode reset -- DECAWM (mode 7) disabled
// ============================================================================

// Disable DECAWM (mode 7), then query it. Should respond with value 2 (reset).
TEST(seq_decrqm_mode_reset)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    output_init(vt);
    state.reset(true);
    callbacks_clear();
    output_clear();

    // Disable DECAWM
    push(vt, "\e[?7l");

    output_clear();
    push(vt, "\e[?7$p");
    // Response: ESC [ ? 7 ; 2 $ y  -- 8 bytes
    ASSERT_OUTPUT_BYTES("\e[?7;2$y", 8);
}

// ============================================================================
// DECRQM: DECOM (mode 6) enabled
// ============================================================================

// Enable DECOM (mode 6), then query it. Should respond with value 1 (set).
TEST(seq_decrqm_decom)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    output_init(vt);
    state.reset(true);
    callbacks_clear();
    output_clear();

    // Enable DECOM
    push(vt, "\e[?6h");

    output_clear();
    push(vt, "\e[?6$p");
    // Response: ESC [ ? 6 ; 1 $ y  -- 8 bytes
    ASSERT_OUTPUT_BYTES("\e[?6;1$y", 8);
}

// ============================================================================
// DECRQM: DECTCEM (mode 25) -- on by default after reset
// ============================================================================

// After reset, DECTCEM (cursor visible, mode 25) is set.
// Query should respond with value 1 (set).
TEST(seq_decrqm_dectcem)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    output_init(vt);
    state.reset(true);
    callbacks_clear();
    output_clear();

    push(vt, "\e[?25$p");
    // Response: ESC [ ? 2 5 ; 1 $ y  -- 9 bytes
    ASSERT_OUTPUT_BYTES("\e[?25;1$y", 9);
}

// ============================================================================
// DECRQM: unknown mode -- responds with value 0 (not recognized)
// ============================================================================

// Query an unknown/unsupported mode number 999. Should respond with
// value 0 (not recognized).
TEST(seq_decrqm_unknown_mode)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    output_init(vt);
    state.reset(true);
    callbacks_clear();
    output_clear();

    push(vt, "\e[?999$p");
    // Response: ESC [ ? 9 9 9 ; 0 $ y  -- 11 bytes
    ASSERT_OUTPUT_BYTES("\e[?999;0$y", 10);
}
