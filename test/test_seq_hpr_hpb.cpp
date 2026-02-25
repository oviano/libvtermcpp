// test_seq_hpr_hpb.cpp -- per-sequence tests for HPR (CSI a) and HPB (CSI j)
//
// HPR -- Horizontal Position Relative (move cursor right by N columns)
// HPB -- Horizontal Position Backward (move cursor left by N columns)

#include "harness.h"

// ============================================================================
// HPR -- CSI a (Horizontal Position Relative)
// ============================================================================

// HPR with default param moves right by 1
TEST(seq_hpr_default_param)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Position cursor at row 1, col 40 (1-based) -> row 0, col 39 (0-based)
    push(vt, "\e[1;40H");
    ASSERT_CURSOR(state, 0, 39);

    // HPR with default param: move right 1
    push(vt, "\e[a");
    ASSERT_CURSOR(state, 0, 40);
}

// HPR with explicit count moves right by that count
TEST(seq_hpr_explicit_count)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Position cursor at row 1, col 40 (1-based) -> row 0, col 39 (0-based)
    push(vt, "\e[1;40H");
    ASSERT_CURSOR(state, 0, 39);

    // HPR 5: move right 5
    push(vt, "\e[5a");
    ASSERT_CURSOR(state, 0, 44);
}

// HPR from near the right edge stays clamped at column 79
TEST(seq_hpr_clamp_right)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Position cursor at row 1, col 80 (1-based) -> row 0, col 79 (0-based)
    push(vt, "\e[1;80H");
    ASSERT_CURSOR(state, 0, 79);

    // HPR 5: should clamp at col 79
    push(vt, "\e[5a");
    ASSERT_CURSOR(state, 0, 79);
}

// ============================================================================
// HPB -- CSI j (Horizontal Position Backward)
// ============================================================================

// HPB with default param moves left by 1
TEST(seq_hpb_default_param)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Position cursor at row 1, col 40 (1-based) -> row 0, col 39 (0-based)
    push(vt, "\e[1;40H");
    ASSERT_CURSOR(state, 0, 39);

    // HPB with default param: move left 1
    push(vt, "\e[j");
    ASSERT_CURSOR(state, 0, 38);
}

// HPB with explicit count moves left by that count
TEST(seq_hpb_explicit_count)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Position cursor at row 1, col 40 (1-based) -> row 0, col 39 (0-based)
    push(vt, "\e[1;40H");
    ASSERT_CURSOR(state, 0, 39);

    // HPB 5: move left 5
    push(vt, "\e[5j");
    ASSERT_CURSOR(state, 0, 34);
}

// HPB from column 0 stays clamped at column 0
TEST(seq_hpb_clamp_left)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Position cursor at row 1, col 1 (1-based) -> row 0, col 0 (0-based)
    push(vt, "\e[1;1H");
    ASSERT_CURSOR(state, 0, 0);

    // HPB 5: should clamp at col 0
    push(vt, "\e[5j");
    ASSERT_CURSOR(state, 0, 0);
}
