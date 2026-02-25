// test_seq_decom.cpp -- tests for Mode 6 (DECOM / Origin Mode)
//
// When DECOM is enabled, cursor positions are relative to the scroll
// region set by DECSTBM (and DECSLRM if active). CUP coordinates are
// relative to the top-left of the scroll region, and cursor movement
// is clamped to the region boundaries.

#include "harness.h"

// ===== Enabling DECOM homes cursor to top of scroll region =====
TEST(seq_decom_enable_homes_cursor)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Set scroll region rows 5..20 (1-based): DECSTBM
    push(vt, "\e[5;20r");

    // Move cursor somewhere else first
    push(vt, "\e[1;1H");
    ASSERT_CURSOR(state, 0, 0);

    // Enable DECOM -- cursor homes to top of scroll region
    push(vt, "\e[?6h");
    // Row 5 (1-based) = row 4 (0-based), col 0
    ASSERT_CURSOR(state, 4, 0);
}

// ===== Disabling DECOM homes cursor to (0,0) =====
TEST(seq_decom_disable_homes_cursor)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Set scroll region and enable DECOM
    push(vt, "\e[5;20r");
    push(vt, "\e[?6h");
    ASSERT_CURSOR(state, 4, 0);

    // Disable DECOM -- cursor homes to absolute (0,0)
    push(vt, "\e[?6l");
    ASSERT_CURSOR(state, 0, 0);
}

// ===== CUP with DECOM is relative to scroll region =====
TEST(seq_decom_cup_relative)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Set scroll region rows 5..20, enable DECOM
    push(vt, "\e[5;20r");
    push(vt, "\e[?6h");

    // CUP(1,1) -- in origin mode, row 1 = top of region = row 4 (0-based)
    push(vt, "\e[1;1H");
    ASSERT_CURSOR(state, 4, 0);

    // CUP(3,3) -- row 3 within region = row 6 (0-based), col 3 = col 2 (0-based)
    push(vt, "\e[3;3H");
    ASSERT_CURSOR(state, 6, 2);
}

// ===== DECOM clamps cursor to scroll region =====
TEST(seq_decom_clamp_to_region)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Set scroll region rows 5..20, enable DECOM
    push(vt, "\e[5;20r");
    push(vt, "\e[?6h");

        // CUP(99,99) -- should clamp to bottom-right of scroll region.
    // Region is rows 5..20 (1-based) = rows 4..19 (0-based).
    // Bottom row = 19, last col = 79.
    push(vt, "\e[99;99H");
    ASSERT_CURSOR(state, 19, 79);
}

// ===== Printing within DECOM region places glyph correctly =====
TEST(seq_decom_print_within_region)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Set scroll region rows 5..20, enable DECOM
    push(vt, "\e[5;20r");
    push(vt, "\e[?6h");

    // Home cursor (to top of region)
    push(vt, "\e[H");
    ASSERT_CURSOR(state, 4, 0);

    callbacks_clear();

    // Print 'A' -- should appear at row 4 (0-based), col 0
    push(vt, "A");
    ASSERT_EQ(g_cb.putglyph_count, 1);
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 'A');
    ASSERT_EQ(g_cb.putglyph[0].row, 4);
    ASSERT_EQ(g_cb.putglyph[0].col, 0);
    ASSERT_CURSOR(state, 4, 1);
}
