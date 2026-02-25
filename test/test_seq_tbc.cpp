// test_seq_tbc.cpp -- per-sequence tests for TBC (CSI g) -- Tab Clear
//
// TBC clears tab stops:
//   CSI 0 g  (or CSI g)  -- clear tab stop at current column
//   CSI 3 g               -- clear all tab stops
//
// Default tab stops are set every 8 columns after reset.

#include "harness.h"

// ============================================================================
// Verify default tab stops after reset
// ============================================================================

// After reset, default tab stops are every 8 columns.
// Tab from col 0 goes to col 8, tab again goes to col 16.
TEST(seq_tbc_verify_default_tabstops)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    ASSERT_CURSOR(state, 0, 0);

    // Tab from col 0 -> col 8
    push(vt, "\t");
    ASSERT_CURSOR(state, 0, 8);

    // Tab from col 8 -> col 16
    push(vt, "\t");
    ASSERT_CURSOR(state, 0, 16);
}

// ============================================================================
// TBC mode 0 -- clear tab stop at current column
// ============================================================================

// Set a custom tab stop at col 5, verify it works, then clear it with
// CSI 0 g, verify the tab now skips to the next default stop at col 8.
TEST(seq_tbc_clear_current_0)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Move to col 5 (1-based col 6) and set HTS there
    push(vt, "\e[6G\eH");

    // Verify: go home, tab should stop at col 5
    push(vt, "\e[G");
    ASSERT_CURSOR(state, 0, 0);
    push(vt, "\t");
    ASSERT_CURSOR(state, 0, 5);

    // Now move to col 5 and clear that tab stop with TBC 0
    push(vt, "\e[6G");
    ASSERT_CURSOR(state, 0, 5);
    push(vt, "\e[0g");

    // Go home and tab again -- should skip col 5 and go to col 8
    push(vt, "\e[G");
    ASSERT_CURSOR(state, 0, 0);
    push(vt, "\t");
    ASSERT_CURSOR(state, 0, 8);
}

// ============================================================================
// TBC default param -- CSI g with no param defaults to mode 0
// ============================================================================

// Same as clear_current but use CSI g (no param) instead of CSI 0 g.
TEST(seq_tbc_default_is_0)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Move to col 5 (1-based col 6) and set HTS there
    push(vt, "\e[6G\eH");

    // Verify: go home, tab should stop at col 5
    push(vt, "\e[G");
    push(vt, "\t");
    ASSERT_CURSOR(state, 0, 5);

    // Move to col 5 and clear with CSI g (default = mode 0)
    push(vt, "\e[6G");
    push(vt, "\e[g");

    // Go home and tab -- should now go to col 8, not col 5
    push(vt, "\e[G");
    push(vt, "\t");
    ASSERT_CURSOR(state, 0, 8);
}

// ============================================================================
// TBC mode 3 -- clear all tab stops
// ============================================================================

// Set a tab stop at col 5, then clear ALL with CSI 3 g.
// Tabbing from col 0 should go all the way to col 79 (last col)
// since there are no tab stops left.
TEST(seq_tbc_clear_all_3)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Set a custom tab at col 5
    push(vt, "\e[6G\eH");

    // Clear ALL tab stops
    push(vt, "\e[3g");

    // Go home, tab -- with no tab stops, should go to last column (79)
    push(vt, "\e[G");
    ASSERT_CURSOR(state, 0, 0);
    push(vt, "\t");
    ASSERT_CURSOR(state, 0, 79);
}
