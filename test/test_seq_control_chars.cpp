// test_seq_control_chars.cpp -- per-sequence tests for C0 control characters
//                               and ESC-initiated control functions
//
// Control characters tested:
//   BEL (0x07) -- bell
//   BS  (0x08) -- backspace
//   HT  (0x09) -- horizontal tab
//   LF  (0x0A) -- line feed
//   VT  (0x0B) -- vertical tab (same as LF)
//   FF  (0x0C) -- form feed (same as LF)
//   CR  (0x0D) -- carriage return
//
// ESC-initiated:
//   ESC D  -- IND (Index) — cursor down, scroll at bottom
//   ESC E  -- NEL (Next Line) — cursor down + col 0
//   ESC H  -- HTS (Horizontal Tab Stop) — set tabstop at current col
//   ESC M  -- RI  (Reverse Index) — cursor up, scroll at top

#include "harness.h"

// ============================================================================
// BEL — bell
// ============================================================================

TEST(seq_control_bel)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    push(vt, "\x07");
    ASSERT_EQ(g_cb.bell_count, 1);
}

// ============================================================================
// BS — backspace overwrites previous character
// ============================================================================

TEST(seq_control_bs)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    Screen& screen = vt.screen();
    screen.enable_altscreen(true);
    screen.set_callbacks(screen_cbs);
    screen.reset(true);
    callbacks_clear();

    // Print "AB", backspace, print "C" -> overwrites 'B' -> row shows "AC"
    push(vt, "AB\x08" "C");
    ASSERT_SCREEN_ROW(vt, screen, 0, "AC");
}

// ============================================================================
// BS at col 0 — clamps to col 0
// ============================================================================

TEST(seq_control_bs_clamp)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Cursor starts at (0,0). BS should keep it at (0,0).
    push(vt, "\x08");
    ASSERT_CURSOR(state, 0, 0);
}

// ============================================================================
// HT — horizontal tab to default tabstop
// ============================================================================

TEST(seq_control_ht)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // From col 0, tab should go to col 8 (default tabstops at every 8)
    push(vt, "\t");
    ASSERT_CURSOR(state, 0, 8);
}

// ============================================================================
// LF — line feed moves cursor down, col unchanged
// ============================================================================

TEST(seq_control_lf)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Move to col 5, then LF -> row 1, col stays 5
    push(vt, "\e[6G"); // CHA 6 = col 5 (0-based)
    push(vt, "\n");
    ASSERT_CURSOR(state, 1, 5);
}

// ============================================================================
// VT — vertical tab (same as LF)
// ============================================================================

TEST(seq_control_vt)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // VT (0x0B) behaves the same as LF
    push(vt, "\x0B");
    ASSERT_CURSOR(state, 1, 0);
}

// ============================================================================
// FF — form feed (same as LF)
// ============================================================================

TEST(seq_control_ff)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // FF (0x0C) behaves the same as LF
    push(vt, "\x0C");
    ASSERT_CURSOR(state, 1, 0);
}

// ============================================================================
// CR — carriage return moves to col 0
// ============================================================================

TEST(seq_control_cr)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Move to col 10 (1-based 11), then CR -> col 0, row unchanged
    push(vt, "\e[11G"); // CHA 11 = col 10 (0-based)
    push(vt, "\r");
    ASSERT_CURSOR(state, 0, 0);
}

// ============================================================================
// IND — ESC D, cursor down one row
// ============================================================================

TEST(seq_control_ind)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // At row 0, IND moves to row 1
    push(vt, "\eD");
    ASSERT_CURSOR(state, 1, 0);
}

// ============================================================================
// NEL — ESC E, cursor down one row and to col 0
// ============================================================================

TEST(seq_control_nel)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Move to col 10, then NEL -> row 1, col 0
    push(vt, "\e[11G"); // CHA 11 = col 10 (0-based)
    push(vt, "\eE");
    ASSERT_CURSOR(state, 1, 0);
}

// ============================================================================
// HTS — ESC H, set horizontal tab stop
// ============================================================================

TEST(seq_control_hts)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Clear all tabstops first
    push(vt, "\e[3g");

    // Move to col 5 (CHA 6 in 1-based) and set a tabstop
    push(vt, "\e[6G");
    ASSERT_CURSOR(state, 0, 5);
    push(vt, "\eH"); // HTS — set tab stop at col 5

    // Go back to col 0, then tab -> should land at col 5
    push(vt, "\e[1G");
    ASSERT_CURSOR(state, 0, 0);
    push(vt, "\t");
    ASSERT_CURSOR(state, 0, 5);
}

// ============================================================================
// RI — ESC M, cursor up one row
// ============================================================================

TEST(seq_control_ri)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Move to row 5, then RI -> row 4
    push(vt, "\e[6;1H"); // CUP row 6, col 1 (1-based) = row 5, col 0
    ASSERT_CURSOR(state, 5, 0);
    push(vt, "\eM");
    ASSERT_CURSOR(state, 4, 0);
}
