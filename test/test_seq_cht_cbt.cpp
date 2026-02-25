// test_seq_cht_cbt.cpp — per-sequence tests for CHT (CSI I) and CBT (CSI Z)
//
// CHT (Cursor Horizontal Tab):     ESC [ <n> I  — advance cursor by <n> tab stops
// CBT (Cursor Backward Tab):       ESC [ <n> Z  — move cursor back by <n> tab stops
//
// Default tab stops are set every 8 columns (0, 8, 16, 24, ...).
// Tests cover default params, explicit counts, edge clamping, and custom
// tab stops set via HTS (ESC H) after clearing all with TBC (CSI 3 g).

#include "harness.h"

// ============================================================================
// CHT — Cursor Horizontal Tab (CSI I)
// ============================================================================

// CHT with no parameter advances the cursor by 1 tab stop
TEST(seq_cht_default_param)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Cursor starts at row 0, col 0 after reset
    ASSERT_CURSOR(state, 0, 0);

    // CHT with no param: advance 1 tab stop -> col 8 (default tabs every 8)
    push(vt, "\e[I");
    ASSERT_CURSOR(state, 0, 8);
}

// CHT with explicit count advances the cursor by that many tab stops
TEST(seq_cht_explicit_count)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    ASSERT_CURSOR(state, 0, 0);

    // CHT 2: advance 2 tab stops -> col 16
    push(vt, "\e[2I");
    ASSERT_CURSOR(state, 0, 16);
}

// CHT near the right edge clamps at col 79
TEST(seq_cht_clamp_right_edge)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Place cursor at row 0, col 75 (CSI 1;76 H — 1-based)
    push(vt, "\e[1;76H");
    ASSERT_CURSOR(state, 0, 75);

    // CHT with a large count: should clamp at col 79
    push(vt, "\e[999I");
    ASSERT_CURSOR(state, 0, 79);
}

// ============================================================================
// CBT — Cursor Backward Tab (CSI Z)
// ============================================================================

// CBT with no parameter moves the cursor back by 1 tab stop
TEST(seq_cbt_default_param)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Place cursor at row 0, col 10 (CSI 1;11 H — 1-based)
    push(vt, "\e[1;11H");
    ASSERT_CURSOR(state, 0, 10);

    // CBT with no param: go back 1 tab stop -> col 8
    push(vt, "\e[Z");
    ASSERT_CURSOR(state, 0, 8);
}

// CBT with explicit count moves the cursor back by that many tab stops
TEST(seq_cbt_explicit_count)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Place cursor at row 0, col 20 (CSI 1;21 H — 1-based)
    push(vt, "\e[1;21H");
    ASSERT_CURSOR(state, 0, 20);

    // CBT 2: go back 2 tab stops -> col 8
    push(vt, "\e[2Z");
    ASSERT_CURSOR(state, 0, 8);
}

// CBT at col 0 stays clamped at col 0
TEST(seq_cbt_clamp_left_edge)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    ASSERT_CURSOR(state, 0, 0);

    // CBT should not move past col 0
    push(vt, "\e[Z");
    ASSERT_CURSOR(state, 0, 0);

    // Even with a large count
    push(vt, "\e[999Z");
    ASSERT_CURSOR(state, 0, 0);
}

// ============================================================================
// CHT with custom tab stops set via HTS
// ============================================================================

// Clear all default tabs, set custom stops at cols 5, 15, 30 via HTS,
// then verify CHT navigates through them correctly
TEST(seq_cht_custom_tabstops)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Clear all tab stops: TBC 3
    push(vt, "\e[3g");

    // Set tab stop at col 5: move cursor to col 5 (CSI 1;6 H), then HTS
    push(vt, "\e[1;6H");
    ASSERT_CURSOR(state, 0, 5);
    push(vt, "\x1bH");

    // Set tab stop at col 15: move cursor to col 15 (CSI 1;16 H), then HTS
    push(vt, "\e[1;16H");
    ASSERT_CURSOR(state, 0, 15);
    push(vt, "\x1bH");

    // Set tab stop at col 30: move cursor to col 30 (CSI 1;31 H), then HTS
    push(vt, "\e[1;31H");
    ASSERT_CURSOR(state, 0, 30);
    push(vt, "\x1bH");

    // Move cursor back to col 0
    push(vt, "\e[1;1H");
    ASSERT_CURSOR(state, 0, 0);

    // CHT 1: col 0 -> col 5
    push(vt, "\e[I");
    ASSERT_CURSOR(state, 0, 5);

    // CHT 1: col 5 -> col 15
    push(vt, "\e[I");
    ASSERT_CURSOR(state, 0, 15);

    // CHT 1: col 15 -> col 30
    push(vt, "\e[I");
    ASSERT_CURSOR(state, 0, 30);
}
