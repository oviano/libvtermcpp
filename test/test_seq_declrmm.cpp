// test_seq_declrmm.cpp -- tests for Mode 69 (DECLRMM / Left-Right Margin Mode)
//
// When DECLRMM is enabled, left/right margins set via DECSLRM (CSI s) take
// effect, bounding ICH and DCH operations. When disabled, the margins are
// stored but have no effect (ICH/DCH use full terminal width).
//
// Note: In libvterm, CSI s is always interpreted as DECSLRM. The mode 69
// flag controls whether the stored margins are actually applied.

#include "harness.h"

// ===== DECLRMM enable: ICH bounded by margins =====
TEST(seq_declrmm_enable)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Enable DECLRMM: CSI ? 69 h
    push(vt, "\e[?69h");

    // Set DECSLRM left=5 right=15 (1-based): CSI 5 ; 15 s
    push(vt, "\e[5;15s");

    // Position cursor within margins at row 3, col 8 (1-based)
    push(vt, "\e[3;8H");
    ASSERT_CURSOR(state, 2, 7);

    callbacks_clear();

    // ICH 1: CSI @
    push(vt, "\e[@");

    // scrollrect should be bounded by right margin (col 15, 1-based = end_col 15 exclusive)
    ASSERT_EQ(g_cb.scrollrect_count, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_row, 2);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_row, 3);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_col, 7);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_col, 15);
    ASSERT_EQ(g_cb.scrollrect[0].downward, 0);
    ASSERT_EQ(g_cb.scrollrect[0].rightward, -1);
}

// ===== DECLRMM disable: margins have no effect on ICH/DCH =====
TEST(seq_declrmm_disable)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Ensure DECLRMM is disabled: CSI ? 69 l
    push(vt, "\e[?69l");

    // Set DECSLRM margins (these get stored but should have no effect)
    push(vt, "\e[10;60s");

    // Position cursor at row 3, col 15 (1-based)
    push(vt, "\e[3;15H");
    ASSERT_CURSOR(state, 2, 14);

    callbacks_clear();

    // ICH 1 -- without DECLRMM, should use full width (end_col=80), not 60
    push(vt, "\e[@");

    ASSERT_EQ(g_cb.scrollrect_count, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_row, 2);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_row, 3);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_col, 14);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_col, 80);
    ASSERT_EQ(g_cb.scrollrect[0].downward, 0);
    ASSERT_EQ(g_cb.scrollrect[0].rightward, -1);
}

// ===== DECLRMM affects ICH: scrollrect columns bounded =====
TEST(seq_declrmm_affects_ich)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Enable DECLRMM and set margins
    push(vt, "\e[?69h");
    push(vt, "\e[10;60s");

    // Position cursor inside margins at row 5, col 20 (1-based)
    push(vt, "\e[5;20H");
    ASSERT_CURSOR(state, 4, 19);

    callbacks_clear();

    // ICH 3
    push(vt, "\e[3@");

    ASSERT_EQ(g_cb.scrollrect_count, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_row, 4);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_row, 5);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_col, 19);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_col, 60);
    ASSERT_EQ(g_cb.scrollrect[0].downward, 0);
    ASSERT_EQ(g_cb.scrollrect[0].rightward, -3);
}

// ===== DECLRMM affects DCH: scrollrect columns bounded =====
TEST(seq_declrmm_affects_dch)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Enable DECLRMM and set margins
    push(vt, "\e[?69h");
    push(vt, "\e[10;60s");

    // Position cursor inside margins at row 5, col 20 (1-based)
    push(vt, "\e[5;20H");
    ASSERT_CURSOR(state, 4, 19);

    callbacks_clear();

    // DCH 2: CSI 2 P
    push(vt, "\e[2P");

    ASSERT_EQ(g_cb.scrollrect_count, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_row, 4);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_row, 5);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_col, 19);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_col, 60);
    ASSERT_EQ(g_cb.scrollrect[0].downward, 0);
    ASSERT_EQ(g_cb.scrollrect[0].rightward, 2);
}
