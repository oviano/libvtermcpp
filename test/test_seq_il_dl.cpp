// test_seq_il_dl.cpp -- per-sequence tests for IL (CSI L) and DL (CSI M)
//
// IL -- Insert Line (CSI Pn L)
//   Inserts Pn blank lines at the cursor row, shifting existing lines
//   downward within the scroll region.  Default Pn is 1.
//   The scrollrect callback receives a negative downward value (content
//   shifts down, i.e. scroll direction is "up" to make room).
//
// DL -- Delete Line (CSI Pn M)
//   Deletes Pn lines starting at the cursor row, shifting lines below
//   upward within the scroll region.  Default Pn is 1.
//   The scrollrect callback receives a positive downward value (content
//   shifts up).
//
// Both operations are bounded by DECSTBM (top/bottom margins) and, when
// enabled, DECSLRM (left/right margins).  If the cursor is outside the
// scroll region, the operation is a no-op.

#include "harness.h"

// ============================================================================
// IL -- Insert Line (CSI L)
// ============================================================================

// IL with no parameter inserts 1 line (downward = -1).
TEST(seq_il_default_param)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Position cursor at row 10, col 0 (CUP 1-based: \e[11;1H)
    push(vt, "\e[11;1H");
    ASSERT_CURSOR(state, 10, 0);

    callbacks_clear();
    push(vt, "\e[L");
    ASSERT_EQ(g_cb.scrollrect_count, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_row, 10);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_row, 25);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_col, 0);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_col, 80);
    ASSERT_EQ(g_cb.scrollrect[0].downward, -1);
    ASSERT_EQ(g_cb.scrollrect[0].rightward, 0);
}

// IL with explicit count of 3 inserts 3 lines (downward = -3).
TEST(seq_il_explicit_count)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Position cursor at row 10, col 0
    push(vt, "\e[11;1H");
    ASSERT_CURSOR(state, 10, 0);

    callbacks_clear();
    push(vt, "\e[3L");
    ASSERT_EQ(g_cb.scrollrect_count, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_row, 10);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_row, 25);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_col, 0);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_col, 80);
    ASSERT_EQ(g_cb.scrollrect[0].downward, -3);
    ASSERT_EQ(g_cb.scrollrect[0].rightward, 0);
}

// IL inside a DECSTBM scroll region is bounded by the bottom margin.
// DECSTBM \e[5;20r sets rows 5-20 (1-based), i.e. 0-based rows 4-19.
// end_row in the scrollrect is 20 (exclusive).
TEST(seq_il_scrollrect_region)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Set DECSTBM: rows 5..20 (1-based)
    push(vt, "\e[5;20r");

    // Position cursor at row 10 (1-based: \e[11;1H => 0-based row 10)
    push(vt, "\e[11;1H");
    ASSERT_CURSOR(state, 10, 0);

    callbacks_clear();
    push(vt, "\e[L");
    ASSERT_EQ(g_cb.scrollrect_count, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_row, 10);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_row, 20);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_col, 0);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_col, 80);
    ASSERT_EQ(g_cb.scrollrect[0].downward, -1);
    ASSERT_EQ(g_cb.scrollrect[0].rightward, 0);
}

// IL with DECSTBM + DECSLRM.
// Enable mode 69 (DECLRMM), set DECSLRM \e[10;60s (cols 10-60, 1-based),
// set DECSTBM \e[5;20r (rows 5-20, 1-based).
// Position cursor inside the margin intersection.
// Scrollrect is bounded by both top/bottom and left/right margins.
TEST(seq_il_with_decslrm)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Enable left/right margin mode
    push(vt, "\e[?69h");
    // Set DECSLRM: cols 10..60 (1-based)
    push(vt, "\e[10;60s");
    // Set DECSTBM: rows 5..20 (1-based)
    push(vt, "\e[5;20r");

    // Position cursor inside margins: row 10, col 15 (1-based: \e[11;16H)
    push(vt, "\e[11;16H");
    ASSERT_CURSOR(state, 10, 15);

    callbacks_clear();
    push(vt, "\e[L");
    ASSERT_EQ(g_cb.scrollrect_count, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_row, 10);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_row, 20);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_col, 9);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_col, 60);
    ASSERT_EQ(g_cb.scrollrect[0].downward, -1);
    ASSERT_EQ(g_cb.scrollrect[0].rightward, 0);
}

// ============================================================================
// DL -- Delete Line (CSI M)
// ============================================================================

// DL with no parameter deletes 1 line (downward = +1).
TEST(seq_dl_default_param)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Position cursor at row 10, col 0 (CUP 1-based: \e[11;1H)
    push(vt, "\e[11;1H");
    ASSERT_CURSOR(state, 10, 0);

    callbacks_clear();
    push(vt, "\e[M");
    ASSERT_EQ(g_cb.scrollrect_count, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_row, 10);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_row, 25);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_col, 0);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_col, 80);
    ASSERT_EQ(g_cb.scrollrect[0].downward, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rightward, 0);
}

// DL with explicit count of 3 deletes 3 lines (downward = +3).
TEST(seq_dl_explicit_count)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Position cursor at row 10, col 0
    push(vt, "\e[11;1H");
    ASSERT_CURSOR(state, 10, 0);

    callbacks_clear();
    push(vt, "\e[3M");
    ASSERT_EQ(g_cb.scrollrect_count, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_row, 10);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_row, 25);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_col, 0);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_col, 80);
    ASSERT_EQ(g_cb.scrollrect[0].downward, 3);
    ASSERT_EQ(g_cb.scrollrect[0].rightward, 0);
}

// DL inside a DECSTBM scroll region is bounded by the bottom margin.
// DECSTBM \e[5;20r sets rows 5-20 (1-based).
// Scrollrect end_row is 20 (exclusive upper bound matching the bottom margin).
TEST(seq_dl_scrollrect_region)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Set DECSTBM: rows 5..20 (1-based)
    push(vt, "\e[5;20r");

    // Position cursor at row 10 (1-based: \e[11;1H => 0-based row 10)
    push(vt, "\e[11;1H");
    ASSERT_CURSOR(state, 10, 0);

    callbacks_clear();
    push(vt, "\e[M");
    ASSERT_EQ(g_cb.scrollrect_count, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_row, 10);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_row, 20);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_col, 0);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_col, 80);
    ASSERT_EQ(g_cb.scrollrect[0].downward, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rightward, 0);
}
