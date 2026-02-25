// test_seq_vpa_vpr_vpb.cpp -- per-sequence tests for VPA (CSI d),
// VPR (CSI e), and VPB (CSI k).

#include "harness.h"

// ============================================================================
// VPA -- Vertical Position Absolute (CSI d)
// ============================================================================

// VPA with no param \e[d goes to row 0
TEST(seq_vpa_default_param)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Move cursor to a known non-zero row first
    push(vt, "\e[10;5H");
    ASSERT_CURSOR(state, 9, 4);

    // VPA with default param -- should go to row 0 (param 1, 1-based)
    push(vt, "\e[d");
    ASSERT_CURSOR(state, 0, 4);
}

// VPA \e[10d goes to row 9 (1-based param 10 -> 0-based row 9)
TEST(seq_vpa_explicit_row)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    push(vt, "\e[1;1H");
    ASSERT_CURSOR(state, 0, 0);

    push(vt, "\e[10d");
    ASSERT_CURSOR(state, 9, 0);
}

// VPA \e[999d clamped to last row (row 24 for a 25-row terminal)
TEST(seq_vpa_clamp_beyond_height)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    push(vt, "\e[999d");
    ASSERT_CURSOR(state, 24, 0);
}

// VPA with DECOM on and DECSTBM set -- position is relative to top margin
TEST(seq_vpa_with_origin_mode)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Set scroll region rows 5-20 (1-based) and enable origin mode
    push(vt, "\e[5;20r");
    push(vt, "\e[?6h");

    // VPA default -- row 1 relative to top margin (row 4 absolute, 0-based)
    push(vt, "\e[d");
    ASSERT_CURSOR(state, 4, 0);

    // VPA row 3 (1-based) relative to margin -> absolute row 6 (0-based)
    push(vt, "\e[3d");
    ASSERT_CURSOR(state, 6, 0);

    // Disable origin mode and reset scroll region
    push(vt, "\e[?6l");
    push(vt, "\e[r");
}

// VPA does not change column
TEST(seq_vpa_preserves_col)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Position cursor at row 5, col 30 (1-based: 6;31)
    push(vt, "\e[6;31H");
    ASSERT_CURSOR(state, 5, 30);

    // VPA to row 15 (1-based) -> row 14 (0-based); col must stay at 30
    push(vt, "\e[15d");
    ASSERT_CURSOR(state, 14, 30);
}

// ============================================================================
// VPR -- Vertical Position Relative (CSI e)
// ============================================================================

// VPR with default param \e[e moves down 1
TEST(seq_vpr_default_param)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Start at row 10, col 5
    push(vt, "\e[11;6H");
    ASSERT_CURSOR(state, 10, 5);

    // VPR default -- move down 1 row
    push(vt, "\e[e");
    ASSERT_CURSOR(state, 11, 5);
}

// VPR from row 24 (last row) clamped at bottom
TEST(seq_vpr_clamp_bottom)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Position cursor at the last row (row 25, 1-based -> row 24, 0-based)
    push(vt, "\e[25;1H");
    ASSERT_CURSOR(state, 24, 0);

    // VPR with large param -- should clamp to row 24
    push(vt, "\e[5e");
    ASSERT_CURSOR(state, 24, 0);
}

// ============================================================================
// VPB -- Vertical Position Backward (CSI k)
// ============================================================================

// VPB with default param \e[k moves up 1
TEST(seq_vpb_default_param)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Start at row 10, col 5
    push(vt, "\e[11;6H");
    ASSERT_CURSOR(state, 10, 5);

    // VPB default -- move up 1 row
    push(vt, "\e[k");
    ASSERT_CURSOR(state, 9, 5);
}

// VPB from row 0 clamped at top
TEST(seq_vpb_clamp_top)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Position cursor at the top-left corner
    push(vt, "\e[1;1H");
    ASSERT_CURSOR(state, 0, 0);

    // VPB with large param -- should clamp to row 0
    push(vt, "\e[5k");
    ASSERT_CURSOR(state, 0, 0);
}
