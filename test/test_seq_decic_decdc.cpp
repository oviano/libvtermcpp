// test_seq_decic_decdc.cpp — per-sequence tests for DECIC and DECDC
//
// DECIC — Insert Column (CSI Ps ' })
//   Inserts Ps columns at the cursor position, shifting existing content to
//   the right within the scroll margins.  Default Ps is 1.  Requires DECLRMM
//   (mode 69) to be active.
//
// DECDC — Delete Column (CSI Ps ' ~)
//   Deletes Ps columns at the cursor position, shifting existing content to
//   the left within the scroll margins.  Default Ps is 1.  Requires DECLRMM
//   (mode 69) to be active.

#include "harness.h"

// ============================================================================
// DECIC tests
// ============================================================================

// DECIC with no parameter defaults to inserting 1 column
TEST(seq_decic_default_param)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Enable DECLRMM (mode 69) and set left/right margins 10..60 (1-based)
    push(vt, "\e[?69h");
    push(vt, "\e[10;60s");

    // Move cursor to row 5, col 20 (0-based) via 1-based CUP
    push(vt, "\e[6;21H");
    ASSERT_CURSOR(state, 5, 20);

    callbacks_clear();

    // DECIC with no param — insert 1 column
    push(vt, "\e['}");

    ASSERT_EQ(g_cb.scrollrect_count, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_row, 0);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_row,   25);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_col, 20);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_col,   60);
    ASSERT_EQ(g_cb.scrollrect[0].downward,  0);
    ASSERT_EQ(g_cb.scrollrect[0].rightward, -1);
}

// DECIC with explicit count inserts that many columns
TEST(seq_decic_explicit_count)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Enable DECLRMM (mode 69) and set left/right margins 10..60 (1-based)
    push(vt, "\e[?69h");
    push(vt, "\e[10;60s");

    // Move cursor to row 5, col 20 (0-based)
    push(vt, "\e[6;21H");
    ASSERT_CURSOR(state, 5, 20);

    callbacks_clear();

    // DECIC 3 — insert 3 columns
    push(vt, "\e[3'}");

    ASSERT_EQ(g_cb.scrollrect_count, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_row, 0);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_row,   25);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_col, 20);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_col,   60);
    ASSERT_EQ(g_cb.scrollrect[0].downward,  0);
    ASSERT_EQ(g_cb.scrollrect[0].rightward, -3);
}

// ============================================================================
// DECDC tests
// ============================================================================

// DECDC with no parameter defaults to deleting 1 column
TEST(seq_decdc_default_param)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Enable DECLRMM (mode 69) and set left/right margins 10..60 (1-based)
    push(vt, "\e[?69h");
    push(vt, "\e[10;60s");

    // Move cursor to row 5, col 20 (0-based)
    push(vt, "\e[6;21H");
    ASSERT_CURSOR(state, 5, 20);

    callbacks_clear();

    // DECDC with no param — delete 1 column
    push(vt, "\e['~");

    ASSERT_EQ(g_cb.scrollrect_count, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_row, 0);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_row,   25);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_col, 20);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_col,   60);
    ASSERT_EQ(g_cb.scrollrect[0].downward,  0);
    ASSERT_EQ(g_cb.scrollrect[0].rightward, 1);
}

// DECDC with explicit count deletes that many columns
TEST(seq_decdc_explicit_count)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Enable DECLRMM (mode 69) and set left/right margins 10..60 (1-based)
    push(vt, "\e[?69h");
    push(vt, "\e[10;60s");

    // Move cursor to row 5, col 20 (0-based)
    push(vt, "\e[6;21H");
    ASSERT_CURSOR(state, 5, 20);

    callbacks_clear();

    // DECDC 3 — delete 3 columns
    push(vt, "\e[3'~");

    ASSERT_EQ(g_cb.scrollrect_count, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_row, 0);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_row,   25);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_col, 20);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_col,   60);
    ASSERT_EQ(g_cb.scrollrect[0].downward,  0);
    ASSERT_EQ(g_cb.scrollrect[0].rightward, 3);
}
