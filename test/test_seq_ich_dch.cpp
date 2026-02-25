// test_seq_ich_dch.cpp -- tests for ICH (CSI @, Insert Character)
//                        and DCH (CSI P, Delete Character)
//
// ICH and DCH generate scrollrect callbacks for the horizontal shift.
// ICH shifts existing content rightward (rightward is negative in scrollrect,
// meaning source is to the right of destination).
// DCH shifts existing content leftward (rightward is positive in scrollrect).

#include "harness.h"


// ===== ICH default param (CSI @) =====
TEST(seq_ich_default_param)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Position cursor at row 5, col 20 (CUP is 1-based)
    push(vt, "\e[6;21H");
    ASSERT_CURSOR(state, 5, 20);

    callbacks_clear();
    push(vt, "\e[@");

    ASSERT_EQ(g_cb.scrollrect_count, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_row, 5);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_row, 6);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_col, 20);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_col, 80);
    ASSERT_EQ(g_cb.scrollrect[0].downward, 0);
    ASSERT_EQ(g_cb.scrollrect[0].rightward, -1);
    ASSERT_CURSOR(state, 5, 20);
}

// ===== ICH explicit count (CSI 5 @) =====
TEST(seq_ich_explicit_count)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Position cursor at row 5, col 20
    push(vt, "\e[6;21H");
    ASSERT_CURSOR(state, 5, 20);

    callbacks_clear();
    push(vt, "\e[5@");

    ASSERT_EQ(g_cb.scrollrect_count, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_row, 5);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_row, 6);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_col, 20);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_col, 80);
    ASSERT_EQ(g_cb.scrollrect[0].downward, 0);
    ASSERT_EQ(g_cb.scrollrect[0].rightward, -5);
    ASSERT_CURSOR(state, 5, 20);
}

// ===== ICH with DECSLRM margins =====
TEST(seq_ich_with_decslrm)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Enable mode 69 (left/right margin mode) and set DECSLRM 10;60
    push(vt, "\e[?69h");
    push(vt, "\e[10;60s");

    // Position cursor inside margins at row 5, col 20
    push(vt, "\e[6;21H");
    ASSERT_CURSOR(state, 5, 20);

    callbacks_clear();
    push(vt, "\e[@");

    // Scrollrect bounded by right margin: col 60 (1-based) = end_col 60 (exclusive)
    ASSERT_EQ(g_cb.scrollrect_count, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_row, 5);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_row, 6);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_col, 20);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_col, 60);
    ASSERT_EQ(g_cb.scrollrect[0].downward, 0);
    ASSERT_EQ(g_cb.scrollrect[0].rightward, -1);
    ASSERT_CURSOR(state, 5, 20);
}

// ===== DCH default param (CSI P) =====
TEST(seq_dch_default_param)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Position cursor at row 5, col 20
    push(vt, "\e[6;21H");
    ASSERT_CURSOR(state, 5, 20);

    callbacks_clear();
    push(vt, "\e[P");

    ASSERT_EQ(g_cb.scrollrect_count, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_row, 5);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_row, 6);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_col, 20);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_col, 80);
    ASSERT_EQ(g_cb.scrollrect[0].downward, 0);
    ASSERT_EQ(g_cb.scrollrect[0].rightward, 1);
    ASSERT_CURSOR(state, 5, 20);
}

// ===== DCH explicit count (CSI 5 P) =====
TEST(seq_dch_explicit_count)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Position cursor at row 5, col 20
    push(vt, "\e[6;21H");
    ASSERT_CURSOR(state, 5, 20);

    callbacks_clear();
    push(vt, "\e[5P");

    ASSERT_EQ(g_cb.scrollrect_count, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_row, 5);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_row, 6);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_col, 20);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_col, 80);
    ASSERT_EQ(g_cb.scrollrect[0].downward, 0);
    ASSERT_EQ(g_cb.scrollrect[0].rightward, 5);
    ASSERT_CURSOR(state, 5, 20);
}

// ===== DCH with DECSLRM margins =====
TEST(seq_dch_with_decslrm)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Enable mode 69 (left/right margin mode) and set DECSLRM 10;60
    push(vt, "\e[?69h");
    push(vt, "\e[10;60s");

    // Position cursor inside margins at row 5, col 20
    push(vt, "\e[6;21H");
    ASSERT_CURSOR(state, 5, 20);

    callbacks_clear();
    push(vt, "\e[P");

    // Scrollrect bounded by right margin: col 60 (1-based) = end_col 60 (exclusive)
    ASSERT_EQ(g_cb.scrollrect_count, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_row, 5);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_row, 6);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_col, 20);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_col, 60);
    ASSERT_EQ(g_cb.scrollrect[0].downward, 0);
    ASSERT_EQ(g_cb.scrollrect[0].rightward, 1);
    ASSERT_CURSOR(state, 5, 20);
}
