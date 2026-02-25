// test_seq_decstbm.cpp -- tests for DECSTBM (CSI r, Set Top and Bottom Margins)
//
// DECSTBM sets the top and bottom margins of the scrolling region.
// CSI parameters are 1-based; ASSERT_CURSOR values are 0-based.

#include "harness.h"

// DECSTBM \e[5;20r sets scroll region rows 5..20 (1-based).
// LF at the bottom margin row should scroll within that region.
TEST(seq_decstbm_set_region)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Set scroll region to rows 5..20 (1-based)
    push(vt, "\e[5;20r");

    // Position cursor at bottom margin row: row 20 (1-based) = row 19 (0-based)
    push(vt, "\e[20;1H");
    ASSERT_CURSOR(state, 19, 0);

    callbacks_clear();

    // LF at bottom margin should trigger scroll within the region
    push(vt, "\n");
    ASSERT_EQ(g_cb.scrollrect_count, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_row, 4);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_row, 20);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_col, 0);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_col, 80);
    ASSERT_EQ(g_cb.scrollrect[0].downward, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rightward, 0);
}

// DECSTBM \e[r with no parameters resets to full screen.
// After setting 5;20, send \e[r, then LF at row 24 generates scrollrect 0..25.
TEST(seq_decstbm_default_params)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Set a restricted scroll region first
    push(vt, "\e[5;20r");

    // Reset to full screen with default params
    push(vt, "\e[r");

    // Position cursor at bottom of full screen: row 25 (1-based) = row 24 (0-based)
    push(vt, "\e[25;1H");
    ASSERT_CURSOR(state, 24, 0);

    callbacks_clear();

    // LF at bottom row should scroll the entire screen
    push(vt, "\n");
    ASSERT_EQ(g_cb.scrollrect_count, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_row, 0);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_row, 25);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_col, 0);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_col, 80);
    ASSERT_EQ(g_cb.scrollrect[0].downward, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rightward, 0);
}

// DECSTBM homes the cursor to (0, 0) when set.
TEST(seq_decstbm_homes_cursor)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Move cursor away from home
    push(vt, "\e[10;20H");
    ASSERT_CURSOR(state, 9, 19);

    // Setting DECSTBM should home cursor
    push(vt, "\e[5;20r");
    ASSERT_CURSOR(state, 0, 0);
}

// With DECOM (origin mode) enabled, DECSTBM homes cursor to the scroll
// region origin (top margin row, column 0).
TEST(seq_decstbm_homes_cursor_with_origin)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Enable DECOM (origin mode) first
    push(vt, "\e[?6h");

    // Set DECSTBM -- with origin mode, cursor goes to scroll region origin
    push(vt, "\e[5;20r");

    // Row 5 (1-based) = row 4 (0-based), col 0
    ASSERT_CURSOR(state, 4, 0);

    // Disable origin mode to leave clean state
    push(vt, "\e[?6l");
}

// DECSTBM \e[5;20r constrains scrolling to the region.
// Position cursor at bottom margin row, LF triggers bounded scrollrect.
TEST(seq_decstbm_scroll_within_region)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Set scroll region to rows 5..20 (1-based)
    push(vt, "\e[5;20r");

    // Move cursor to bottom margin row: row 20 (1-based) = row 19 (0-based)
    push(vt, "\e[20;1H");
    ASSERT_CURSOR(state, 19, 0);

    callbacks_clear();

    // LF at bottom margin triggers scroll within the region
    push(vt, "\n");
    ASSERT_EQ(g_cb.scrollrect_count, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_row, 4);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_row, 20);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_col, 0);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_col, 80);
    ASSERT_EQ(g_cb.scrollrect[0].downward, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rightward, 0);

    // Cursor stays at the bottom margin row
    ASSERT_CURSOR(state, 19, 0);
}
