// test_seq_su_sd.cpp -- tests for SU (CSI S, Scroll Up) and SD (CSI T, Scroll Down)
//
// SU (CSI S) scrolls content up (downward is positive).
// SD (CSI T) scrolls content down (downward is negative).

#include "harness.h"


// --------------------------------------------------------------------------
// SU -- Scroll Up (CSI S)
// --------------------------------------------------------------------------

// SU with default parameter scrolls entire screen up by 1
TEST(seq_su_default_param)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    push(vt, "\e[S");

    ASSERT_EQ(g_cb.scrollrect_count, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_row, 0);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_row,   25);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_col, 0);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_col,   80);
    ASSERT_EQ(g_cb.scrollrect[0].downward,  1);
    ASSERT_EQ(g_cb.scrollrect[0].rightward, 0);
}

// SU with explicit count scrolls up by that many lines
TEST(seq_su_explicit_count)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    push(vt, "\e[3S");

    ASSERT_EQ(g_cb.scrollrect_count, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_row, 0);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_row,   25);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_col, 0);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_col,   80);
    ASSERT_EQ(g_cb.scrollrect[0].downward,  3);
    ASSERT_EQ(g_cb.scrollrect[0].rightward, 0);
}

// SU with DECSTBM bounds the scroll region to top/bottom margins
TEST(seq_su_with_decstbm)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // DECSTBM: rows 5..20 (1-based) => start_row=4, end_row=20 (0-based exclusive)
    push(vt, "\e[5;20r");
    callbacks_clear();

    push(vt, "\e[S");

    ASSERT_EQ(g_cb.scrollrect_count, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_row, 4);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_row,   20);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_col, 0);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_col,   80);
    ASSERT_EQ(g_cb.scrollrect[0].downward,  1);
    ASSERT_EQ(g_cb.scrollrect[0].rightward, 0);
}

// SU with DECSTBM + DECSLRM bounds scroll region by all four margins
TEST(seq_su_with_decslrm)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Enable DECLRMM (mode 69) for left/right margins
    push(vt, "\e[?69h");
    // DECSTBM: rows 5..20 (1-based) => start_row=4, end_row=20
    // DECSLRM: cols 10..60 (1-based) => start_col=9, end_col=60
    push(vt, "\e[5;20r");
    push(vt, "\e[10;60s");
    callbacks_clear();

    push(vt, "\e[S");

    ASSERT_EQ(g_cb.scrollrect_count, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_row, 4);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_row,   20);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_col, 9);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_col,   60);
    ASSERT_EQ(g_cb.scrollrect[0].downward,  1);
    ASSERT_EQ(g_cb.scrollrect[0].rightward, 0);
}


// --------------------------------------------------------------------------
// SD -- Scroll Down (CSI T)
// --------------------------------------------------------------------------

// SD with default parameter scrolls entire screen down by 1
TEST(seq_sd_default_param)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    push(vt, "\e[T");

    ASSERT_EQ(g_cb.scrollrect_count, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_row, 0);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_row,   25);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_col, 0);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_col,   80);
    ASSERT_EQ(g_cb.scrollrect[0].downward,  -1);
    ASSERT_EQ(g_cb.scrollrect[0].rightward, 0);
}

// SD with explicit count scrolls down by that many lines
TEST(seq_sd_explicit_count)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    push(vt, "\e[3T");

    ASSERT_EQ(g_cb.scrollrect_count, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_row, 0);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_row,   25);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_col, 0);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_col,   80);
    ASSERT_EQ(g_cb.scrollrect[0].downward,  -3);
    ASSERT_EQ(g_cb.scrollrect[0].rightward, 0);
}

// SD with DECSTBM bounds the scroll region to top/bottom margins
TEST(seq_sd_with_decstbm)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // DECSTBM: rows 5..20 (1-based) => start_row=4, end_row=20 (0-based exclusive)
    push(vt, "\e[5;20r");
    callbacks_clear();

    push(vt, "\e[T");

    ASSERT_EQ(g_cb.scrollrect_count, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_row, 4);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_row,   20);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_col, 0);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_col,   80);
    ASSERT_EQ(g_cb.scrollrect[0].downward,  -1);
    ASSERT_EQ(g_cb.scrollrect[0].rightward, 0);
}
