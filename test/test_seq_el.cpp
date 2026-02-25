// test_seq_el.cpp -- per-sequence tests for EL (CSI K) and DECSEL (CSI ? K)
//
// EL -- Erase in Line (CSI Ps K)
//   Ps = 0  Erase from cursor to end of line (default).
//   Ps = 1  Erase from start of line to cursor (inclusive).
//   Ps = 2  Erase entire line.
//
// DECSEL -- Selective Erase in Line (CSI ? Ps K)
//   Same as EL but sets the selective flag, so only erasable characters
//   (those not protected by DECSCA) are cleared.

#include "harness.h"

// ============================================================================
// EL 0 -- Erase Right (cursor to end of line)
// ============================================================================

// EL 0: erase from cursor column to end of line
TEST(seq_el_0_erase_right)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Position cursor at row 5, col 20 (1-based: 6;21)
    push(vt, "\e[6;21H");
    ASSERT_CURSOR(state, 5, 20);

    callbacks_clear();
    push(vt, "\e[0K");
    ASSERT_EQ(g_cb.erase_count, 1);
    ASSERT_EQ(g_cb.erase[0].rect.start_row, 5);
    ASSERT_EQ(g_cb.erase[0].rect.end_row, 6);
    ASSERT_EQ(g_cb.erase[0].rect.start_col, 20);
    ASSERT_EQ(g_cb.erase[0].rect.end_col, 80);
    ASSERT_EQ(g_cb.erase[0].selective, false);
}

// ============================================================================
// EL 1 -- Erase Left (start of line to cursor, inclusive)
// ============================================================================

// EL 1: erase from start of line through cursor column (inclusive)
TEST(seq_el_1_erase_left)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Position cursor at row 5, col 20 (1-based: 6;21)
    push(vt, "\e[6;21H");
    ASSERT_CURSOR(state, 5, 20);

    callbacks_clear();
    push(vt, "\e[1K");
    ASSERT_EQ(g_cb.erase_count, 1);
    ASSERT_EQ(g_cb.erase[0].rect.start_row, 5);
    ASSERT_EQ(g_cb.erase[0].rect.end_row, 6);
    ASSERT_EQ(g_cb.erase[0].rect.start_col, 0);
    ASSERT_EQ(g_cb.erase[0].rect.end_col, 21); // inclusive: col+1
    ASSERT_EQ(g_cb.erase[0].selective, false);
}

// ============================================================================
// EL 2 -- Erase Entire Line
// ============================================================================

// EL 2: erase from column 0 to column 80 (full line width)
TEST(seq_el_2_erase_entire_line)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Position cursor at row 5, col 20 (1-based: 6;21)
    push(vt, "\e[6;21H");
    ASSERT_CURSOR(state, 5, 20);

    callbacks_clear();
    push(vt, "\e[2K");
    ASSERT_EQ(g_cb.erase_count, 1);
    ASSERT_EQ(g_cb.erase[0].rect.start_row, 5);
    ASSERT_EQ(g_cb.erase[0].rect.end_row, 6);
    ASSERT_EQ(g_cb.erase[0].rect.start_col, 0);
    ASSERT_EQ(g_cb.erase[0].rect.end_col, 80);
    ASSERT_EQ(g_cb.erase[0].selective, false);
}

// ============================================================================
// EL default -- no parameter is same as EL 0
// ============================================================================

// CSI K with no parameter behaves identically to CSI 0 K
TEST(seq_el_default_is_0)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Position cursor at row 5, col 20 (1-based: 6;21)
    push(vt, "\e[6;21H");
    ASSERT_CURSOR(state, 5, 20);

    callbacks_clear();
    push(vt, "\e[K");
    ASSERT_EQ(g_cb.erase_count, 1);
    ASSERT_EQ(g_cb.erase[0].rect.start_row, 5);
    ASSERT_EQ(g_cb.erase[0].rect.end_row, 6);
    ASSERT_EQ(g_cb.erase[0].rect.start_col, 20);
    ASSERT_EQ(g_cb.erase[0].rect.end_col, 80);
    ASSERT_EQ(g_cb.erase[0].selective, false);
}

// ============================================================================
// EL 0 at column 0 -- erases entire line
// ============================================================================

// When cursor is at column 0, EL 0 erases from col 0..80 (the full line)
TEST(seq_el_at_col_0)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Position cursor at row 5, col 0 (1-based: 6;1)
    push(vt, "\e[6;1H");
    ASSERT_CURSOR(state, 5, 0);

    callbacks_clear();
    push(vt, "\e[0K");
    ASSERT_EQ(g_cb.erase_count, 1);
    ASSERT_EQ(g_cb.erase[0].rect.start_row, 5);
    ASSERT_EQ(g_cb.erase[0].rect.end_row, 6);
    ASSERT_EQ(g_cb.erase[0].rect.start_col, 0);
    ASSERT_EQ(g_cb.erase[0].rect.end_col, 80);
    ASSERT_EQ(g_cb.erase[0].selective, false);
}

// ============================================================================
// EL 1 at last column -- erases entire line
// ============================================================================

// When cursor is at col 79 (last), EL 1 erases col 0..80 (the full line)
TEST(seq_el_at_last_col)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Position cursor at row 5, col 79 (1-based: 6;80)
    push(vt, "\e[6;80H");
    ASSERT_CURSOR(state, 5, 79);

    callbacks_clear();
    push(vt, "\e[1K");
    ASSERT_EQ(g_cb.erase_count, 1);
    ASSERT_EQ(g_cb.erase[0].rect.start_row, 5);
    ASSERT_EQ(g_cb.erase[0].rect.end_row, 6);
    ASSERT_EQ(g_cb.erase[0].rect.start_col, 0);
    ASSERT_EQ(g_cb.erase[0].rect.end_col, 80); // col 79 + 1 = 80
    ASSERT_EQ(g_cb.erase[0].selective, false);
}

// ============================================================================
// DECSEL -- Selective Erase in Line (CSI ? K)
// ============================================================================

// DECSEL sets the selective flag in the erase callback
TEST(seq_decsel_selective)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Position cursor at row 5, col 20 (1-based: 6;21)
    push(vt, "\e[6;21H");
    ASSERT_CURSOR(state, 5, 20);

    callbacks_clear();
    push(vt, "\e[?K");
    ASSERT_EQ(g_cb.erase_count, 1);
    ASSERT_EQ(g_cb.erase[0].rect.start_row, 5);
    ASSERT_EQ(g_cb.erase[0].rect.end_row, 6);
    ASSERT_EQ(g_cb.erase[0].rect.start_col, 20);
    ASSERT_EQ(g_cb.erase[0].rect.end_col, 80);
    ASSERT_EQ(g_cb.erase[0].selective, true);
}
