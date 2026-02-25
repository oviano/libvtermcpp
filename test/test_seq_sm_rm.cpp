// test_seq_sm_rm.cpp -- tests for SM (CSI h) / RM (CSI l) and IRM (Insert/Replace Mode)
//
// IRM (mode 4) controls whether printing characters insert (shifting
// existing content right) or replace at the cursor position.
// LNM (mode 20) controls whether LF also performs CR, and whether
// pressing Enter sends CR+LF instead of just CR.

#include "harness.h"

// ===== IRM insert mode: printing shifts existing content right =====
TEST(seq_sm_irm_insert_mode)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Print "AB" at the start of the line
    push(vt, "AB");
    ASSERT_CURSOR(state, 0, 2);

    // Enable IRM (insert mode): CSI 4 h
    push(vt, "\e[4h");

    // Move cursor back to col 0
    push(vt, "\e[G");
    ASSERT_CURSOR(state, 0, 0);

    callbacks_clear();

    // Print 'X' in insert mode -- should generate a scrollrect for the shift
    push(vt, "X");

    // scrollrect should show rightward shift of -1 (insert 1 column)
    ASSERT_EQ(g_cb.scrollrect_count, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_row, 0);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_row, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_col, 0);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_col, 80);
    ASSERT_EQ(g_cb.scrollrect[0].downward, 0);
    ASSERT_EQ(g_cb.scrollrect[0].rightward, -1);

    // putglyph 'X' at col 0
    ASSERT_EQ(g_cb.putglyph_count, 1);
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 'X');
    ASSERT_EQ(g_cb.putglyph[0].row, 0);
    ASSERT_EQ(g_cb.putglyph[0].col, 0);
    ASSERT_CURSOR(state, 0, 1);
}

// ===== IRM replace mode: printing overwrites without scrollrect =====
TEST(seq_rm_irm_replace_mode)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Print "AB" at the start
    push(vt, "AB");

    // Ensure replace mode (default, but explicitly disable IRM): CSI 4 l
    push(vt, "\e[4l");

    // Move cursor back to col 0
    push(vt, "\e[G");
    ASSERT_CURSOR(state, 0, 0);

    callbacks_clear();

    // Print 'X' in replace mode -- should NOT generate any scrollrect
    push(vt, "X");

    ASSERT_EQ(g_cb.scrollrect_count, 0);
    // putglyph 'X' at col 0 -- just overwrites
    ASSERT_EQ(g_cb.putglyph_count, 1);
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 'X');
    ASSERT_EQ(g_cb.putglyph[0].row, 0);
    ASSERT_EQ(g_cb.putglyph[0].col, 0);
    ASSERT_CURSOR(state, 0, 1);
}

// ===== LNM mode: pressing Enter sends CR+LF when enabled =====
TEST(seq_sm_lnm_newline_mode)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    output_init(vt);
    state.reset(true);
    callbacks_clear();

    // Without LNM, Enter sends just CR
    output_clear();
    vt.keyboard_key(Key::Enter, Modifier::None);
    ASSERT_OUTPUT_BYTES("\x0d", 1);

    // Enable LNM: CSI 20 h
    push(vt, "\e[20h");

    // With LNM, Enter sends CR+LF
    output_clear();
    vt.keyboard_key(Key::Enter, Modifier::None);
    ASSERT_OUTPUT_BYTES("\x0d\x0a", 2);
}

// ===== IRM insert shifts right at cursor position =====
TEST(seq_irm_insert_shifts_right)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Print some text
    push(vt, "HELLO");

    // Enable insert mode
    push(vt, "\e[4h");

    // Move cursor to col 2
    push(vt, "\e[3G");
    ASSERT_CURSOR(state, 0, 2);

    callbacks_clear();

    // Print 'Z' in insert mode at col 2
    push(vt, "Z");

    // scrollrect shifts from col 2 rightward by -1
    ASSERT_EQ(g_cb.scrollrect_count, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_row, 0);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_row, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_col, 2);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_col, 80);
    ASSERT_EQ(g_cb.scrollrect[0].downward, 0);
    ASSERT_EQ(g_cb.scrollrect[0].rightward, -1);

    // 'Z' placed at col 2
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 'Z');
    ASSERT_EQ(g_cb.putglyph[0].row, 0);
    ASSERT_EQ(g_cb.putglyph[0].col, 2);
    ASSERT_CURSOR(state, 0, 3);
}
