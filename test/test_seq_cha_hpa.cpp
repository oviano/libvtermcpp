// test_seq_cha_hpa.cpp — per-sequence tests for CHA (CSI G) and HPA (CSI `)
//
// CHA — Cursor Character Absolute (CSI Ps G)
//   Moves cursor to column Ps (1-based). Default is 1.
//
// HPA — Horizontal Position Absolute (CSI Ps `)
//   Moves cursor to column Ps (1-based). Default is 1.

#include "harness.h"

// ============================================================================
// CHA tests
// ============================================================================

// CHA with no parameter defaults to column 1 (0-based col 0)
TEST(seq_cha_default_param)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Move cursor to row 10, col 40 (1-based) so we start away from origin
    push(vt, "\e[10;40H");
    ASSERT_CURSOR(state, 9, 39);

    // CHA with no param — should move to col 0
    push(vt, "\e[G");
    ASSERT_CURSOR(state, 9, 0);
}

// CHA with explicit column parameter (1-based 20 -> 0-based 19)
TEST(seq_cha_explicit_col)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    push(vt, "\e[10;40H");
    ASSERT_CURSOR(state, 9, 39);

    // CHA to column 20 (1-based) -> col 19 (0-based)
    push(vt, "\e[20G");
    ASSERT_CURSOR(state, 9, 19);
}

// CHA with column beyond terminal width is clamped to last column (79)
TEST(seq_cha_clamp_beyond_width)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    push(vt, "\e[10;40H");
    ASSERT_CURSOR(state, 9, 39);

    // CHA to column 999 — should clamp to col 79
    push(vt, "\e[999G");
    ASSERT_CURSOR(state, 9, 79);
}

// CHA does not change the current row
TEST(seq_cha_preserves_row)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    push(vt, "\e[15;50H");
    ASSERT_CURSOR(state, 14, 49);

    // CHA to column 5 — row must remain 14 (0-based)
    push(vt, "\e[5G");
    ASSERT_CURSOR(state, 14, 4);

    // CHA with default — row must still remain 14
    push(vt, "\e[G");
    ASSERT_CURSOR(state, 14, 0);
}

// ============================================================================
// HPA tests
// ============================================================================

// HPA with no parameter defaults to column 1 (0-based col 0)
TEST(seq_hpa_default_param)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    push(vt, "\e[10;40H");
    ASSERT_CURSOR(state, 9, 39);

    // HPA with no param — should move to col 0
    push(vt, "\e[`");
    ASSERT_CURSOR(state, 9, 0);
}

// HPA with explicit column parameter (1-based 20 -> 0-based 19)
TEST(seq_hpa_explicit_col)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    push(vt, "\e[10;40H");
    ASSERT_CURSOR(state, 9, 39);

    // HPA to column 20 (1-based) -> col 19 (0-based)
    push(vt, "\e[20`");
    ASSERT_CURSOR(state, 9, 19);
}

// HPA with column beyond terminal width is clamped to last column (79)
TEST(seq_hpa_clamp_beyond_width)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    push(vt, "\e[10;40H");
    ASSERT_CURSOR(state, 9, 39);

    // HPA to column 999 — should clamp to col 79
    push(vt, "\e[999`");
    ASSERT_CURSOR(state, 9, 79);
}
