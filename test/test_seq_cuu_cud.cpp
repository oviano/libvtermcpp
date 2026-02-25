// test_seq_cuu_cud.cpp — per-sequence tests for CUU (CSI A) and CUD (CSI B)
//
// CUU (Cursor Up):    ESC [ Pn A   — move cursor up by Pn rows
// CUD (Cursor Down):  ESC [ Pn B   — move cursor down by Pn rows
//
// Per ECMA-48, a parameter value of 0 is treated as 1.
// Per DEC spec, CUU/CUD stop at the scroll-region margin when the cursor
// is inside the scroll region, but stop at the screen edge when the cursor
// is outside the scroll region.
//
// In libvterm, scroll-region clamping for CUU/CUD is tied to DECOM (origin
// mode).  When DECOM is set (\e[?6h), cursor positions are clamped to the
// scroll region after each CSI command.  When DECOM is reset (the default),
// positions are clamped to the full screen.

#include "harness.h"

// ============================================================================
// CUU — Cursor Up (CSI A)
// ============================================================================

// CUU with no parameter moves the cursor up by 1 row.
TEST(seq_cuu_default_param)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Position cursor at row 5, col 0 (CUP uses 1-based: \e[6;1H)
    push(vt, "\e[6;1H");
    ASSERT_CURSOR(state, 5, 0);

    // CUU with no param — should move up 1 row
    push(vt, "\e[A");
    ASSERT_CURSOR(state, 4, 0);
}

// CUU with an explicit count moves the cursor up by that many rows.
TEST(seq_cuu_explicit_param)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Position cursor at row 10, col 3 (CUP: \e[11;4H)
    push(vt, "\e[11;4H");
    ASSERT_CURSOR(state, 10, 3);

    // CUU 5 — should move up 5 rows to row 5
    push(vt, "\e[5A");
    ASSERT_CURSOR(state, 5, 3);
}

// CUU with parameter 0 is treated as 1 per ECMA-48.
TEST(seq_cuu_zero_param)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Position cursor at row 5, col 0 (CUP: \e[6;1H)
    push(vt, "\e[6;1H");
    ASSERT_CURSOR(state, 5, 0);

    // CUU 0 — should be treated as CUU 1, move up 1 row
    push(vt, "\e[0A");
    ASSERT_CURSOR(state, 4, 0);
}

// CUU at row 0 clamps and stays at row 0 (top screen edge).
TEST(seq_cuu_clamp_top_edge)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Position cursor at row 0, col 5 (CUP: \e[1;6H)
    push(vt, "\e[1;6H");
    ASSERT_CURSOR(state, 0, 5);

    // CUU 1 — already at top, should stay at row 0
    push(vt, "\e[A");
    ASSERT_CURSOR(state, 0, 5);

    // CUU with a large count — should still clamp to row 0
    push(vt, "\e[999A");
    ASSERT_CURSOR(state, 0, 5);
}

// CUU inside a DECSTBM scroll region stops at the top margin.
//
// Origin mode (DECOM, \e[?6h) must be set so that the cursor is clamped
// to the scroll region rather than the full screen.  With DECOM set, CUP
// coordinates are relative to the scroll region origin.
TEST(seq_cuu_stops_at_scroll_top)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Set scroll region: rows 5-15 (1-based: \e[5;15r) => 0-based rows 4-14
    push(vt, "\e[5;15r");

    // Enable origin mode — cursor positions are now relative to the scroll region
    push(vt, "\e[?6h");

    // With DECOM set, CUP \e[6;1H means row 6 within the scroll region,
    // i.e. 0-based row = scrollregion_top + (6-1) = 4 + 5 = 9.
    push(vt, "\e[6;1H");
    ASSERT_CURSOR(state, 9, 0);

    // CUU 20 — should stop at the top margin (row 4, 0-based)
    push(vt, "\e[20A");
    ASSERT_CURSOR(state, 4, 0);
}

// CUU from outside (above) the scroll region ignores the scroll margin
// and stops at the screen edge (row 0).
//
// When DECOM is off (the default), the cursor is not constrained by the
// scroll margins, so CUU simply clamps to row 0.
TEST(seq_cuu_passes_scroll_top_from_outside)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Set scroll region: rows 10-20 (1-based: \e[10;20r) => 0-based rows 9-19
    push(vt, "\e[10;20r");

    // Position cursor at row 2, col 0 (above scroll region; CUP: \e[3;1H).
    // DECOM is off, so CUP addresses the full screen.
    push(vt, "\e[3;1H");
    ASSERT_CURSOR(state, 2, 0);

    // CUU 20 — should pass through the scroll top margin and stop at row 0
    push(vt, "\e[20A");
    ASSERT_CURSOR(state, 0, 0);
}

// ============================================================================
// CUD — Cursor Down (CSI B)
// ============================================================================

// CUD with no parameter moves the cursor down by 1 row.
TEST(seq_cud_default_param)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Position cursor at row 5, col 0 (CUP: \e[6;1H)
    push(vt, "\e[6;1H");
    ASSERT_CURSOR(state, 5, 0);

    // CUD with no param — should move down 1 row
    push(vt, "\e[B");
    ASSERT_CURSOR(state, 6, 0);
}

// CUD at the last row clamps and stays at the bottom screen edge.
TEST(seq_cud_clamp_bottom_edge)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Position cursor at last row (row 24, 0-based; CUP: \e[25;1H)
    push(vt, "\e[25;1H");
    ASSERT_CURSOR(state, 24, 0);

    // CUD 1 — already at bottom, should stay at row 24
    push(vt, "\e[B");
    ASSERT_CURSOR(state, 24, 0);

    // CUD with a large count — should still clamp to row 24
    push(vt, "\e[999B");
    ASSERT_CURSOR(state, 24, 0);
}

// CUD inside a DECSTBM scroll region stops at the bottom margin.
//
// Origin mode (DECOM) must be set for the cursor to be clamped to the
// scroll region.  With DECOM set, CUP row coordinates are relative to
// the top of the scroll region.
TEST(seq_cud_stops_at_scroll_bottom)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Set scroll region: rows 5-15 (1-based: \e[5;15r) => 0-based rows 4-14
    push(vt, "\e[5;15r");

    // Enable origin mode
    push(vt, "\e[?6h");

    // With DECOM set, CUP \e[6;1H means row 6 within the scroll region,
    // i.e. 0-based row = scrollregion_top + (6-1) = 4 + 5 = 9.
    push(vt, "\e[6;1H");
    ASSERT_CURSOR(state, 9, 0);

    // CUD 20 — should stop at the bottom margin (row 14, 0-based)
    push(vt, "\e[20B");
    ASSERT_CURSOR(state, 14, 0);
}
