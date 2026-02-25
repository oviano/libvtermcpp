// test_seq_decslrm.cpp — tests for DECSLRM (CSI Ps ; Ps s)
//
// DECSLRM — Set Left and Right Margins
//   Requires DECLRMM (mode 69) to be enabled.  Without mode 69, CSI s is
//   SCOSC (Save Cursor, SCO style) instead.
//
//   CSI Pl ; Pr s   sets the left margin to Pl and the right margin to Pr
//                    (1-based).  Omitting both parameters resets to full width.
//   Setting DECSLRM homes the cursor.

#include "harness.h"

// ============================================================================
// DECSLRM tests
// ============================================================================

// DECSLRM sets left/right scroll region — verify via ICH scrollrect bounds
TEST(seq_decslrm_set_region)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Enable DECLRMM (mode 69)
    push(vt, "\e[?69h");

    // Set left margin 10, right margin 60 (1-based)
    push(vt, "\e[10;60s");

    // Move cursor inside the margins: row 5, col 20 (1-based)
    push(vt, "\e[5;20H");
    ASSERT_CURSOR(state, 4, 19);

    callbacks_clear();

    // ICH — insert 1 character
    push(vt, "\e[@");

    // scrollrect should be bounded by the left and right margins
    ASSERT_EQ(g_cb.scrollrect_count, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_row, 4);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_row, 5);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_col, 19);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_col, 60);
    ASSERT_EQ(g_cb.scrollrect[0].downward, 0);
    ASSERT_EQ(g_cb.scrollrect[0].rightward, -1);
}

// DECSLRM with no parameters resets to full width
TEST(seq_decslrm_default_params)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Enable DECLRMM (mode 69)
    push(vt, "\e[?69h");

    // Set margins first
    push(vt, "\e[10;60s");

    // Reset margins by sending CSI s with no parameters
    push(vt, "\e[s");

    // Move cursor to a known position
    push(vt, "\e[5;20H");
    ASSERT_CURSOR(state, 4, 19);

    callbacks_clear();

    // ICH — should now use full width since margins were reset
    push(vt, "\e[@");

    ASSERT_EQ(g_cb.scrollrect_count, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_row, 4);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_row, 5);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_col, 19);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_col, 80);
    ASSERT_EQ(g_cb.scrollrect[0].downward, 0);
    ASSERT_EQ(g_cb.scrollrect[0].rightward, -1);
}

// Setting DECSLRM homes the cursor
TEST(seq_decslrm_homes_cursor)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Enable DECLRMM (mode 69)
    push(vt, "\e[?69h");

    // Move cursor away from home
    push(vt, "\e[10;40H");
    ASSERT_CURSOR(state, 9, 39);

    // Set DECSLRM — should home the cursor
    push(vt, "\e[10;60s");

    ASSERT_CURSOR(state, 0, 0);
}

// Without mode 69, CSI s is SCOSC not DECSLRM — margins are not set
TEST(seq_decslrm_requires_mode69)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Do NOT enable mode 69 — CSI 10;60 s should be treated as SCOSC
    push(vt, "\e[10;60s");

    // Move cursor inside what would have been the margin region
    push(vt, "\e[5;20H");
    ASSERT_CURSOR(state, 4, 19);

    callbacks_clear();

    // ICH — should use full width (no left/right margins active)
    push(vt, "\e[@");

    ASSERT_EQ(g_cb.scrollrect_count, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_row, 4);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_row, 5);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_col, 19);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_col, 80);
    ASSERT_EQ(g_cb.scrollrect[0].downward, 0);
    ASSERT_EQ(g_cb.scrollrect[0].rightward, -1);
}

// With DECOM + mode 69, cursor homes to margin origin
TEST(seq_decslrm_homes_with_origin)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Set DECSTBM: rows 5..15 (1-based)
    push(vt, "\e[5;15r");

    // Enable DECOM (origin mode)
    push(vt, "\e[?6h");

    // Enable DECLRMM (mode 69)
    push(vt, "\e[?69h");

    // Set DECSLRM: cols 20..60 (1-based)
    push(vt, "\e[20;60s");

        // With DECOM, cursor should home to the top-left of the margin area:
    // row 4 (0-based for DECSTBM top=5), col 19 (0-based for DECSLRM left=20)
    ASSERT_CURSOR(state, 4, 19);
}
