// test_seq_ed.cpp -- tests for ED (CSI J, Erase in Display)
//                  and DECSED (CSI ?J, Selective Erase in Display)

#include "harness.h"


// ===== ED 0: Erase below (from cursor to end of display) =====
TEST(seq_ed_0_erase_below)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Position cursor at row 10, col 5 (CSI params are 1-based)
    push(vt, "\e[11;6H");
    ASSERT_CURSOR(state, 10, 5);

    callbacks_clear();
    push(vt, "\e[0J");

        // Expect two erase rects:
    //   [0] remainder of current line: row 10..11, col 5..80
    //   [1] rows below: rows 11..25, cols 0..80
    ASSERT_EQ(g_cb.erase_count, 2);

    ASSERT_EQ(g_cb.erase[0].rect.start_row, 10);
    ASSERT_EQ(g_cb.erase[0].rect.end_row, 11);
    ASSERT_EQ(g_cb.erase[0].rect.start_col, 5);
    ASSERT_EQ(g_cb.erase[0].rect.end_col, 80);
    ASSERT_EQ(g_cb.erase[0].selective, false);

    ASSERT_EQ(g_cb.erase[1].rect.start_row, 11);
    ASSERT_EQ(g_cb.erase[1].rect.end_row, 25);
    ASSERT_EQ(g_cb.erase[1].rect.start_col, 0);
    ASSERT_EQ(g_cb.erase[1].rect.end_col, 80);
    ASSERT_EQ(g_cb.erase[1].selective, false);

    // Cursor does not move
    ASSERT_CURSOR(state, 10, 5);
}


// ===== ED 1: Erase above (from start of display to cursor) =====
TEST(seq_ed_1_erase_above)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Position cursor at row 10, col 5
    push(vt, "\e[11;6H");
    ASSERT_CURSOR(state, 10, 5);

    callbacks_clear();
    push(vt, "\e[1J");

        // Expect two erase rects:
    //   [0] rows above: rows 0..10, cols 0..80
    //   [1] start of current line up to and including cursor col:
    //       row 10..11, col 0..6 (end_col is exclusive, cursor is at 5)
    ASSERT_EQ(g_cb.erase_count, 2);

    ASSERT_EQ(g_cb.erase[0].rect.start_row, 0);
    ASSERT_EQ(g_cb.erase[0].rect.end_row, 10);
    ASSERT_EQ(g_cb.erase[0].rect.start_col, 0);
    ASSERT_EQ(g_cb.erase[0].rect.end_col, 80);
    ASSERT_EQ(g_cb.erase[0].selective, false);

    ASSERT_EQ(g_cb.erase[1].rect.start_row, 10);
    ASSERT_EQ(g_cb.erase[1].rect.end_row, 11);
    ASSERT_EQ(g_cb.erase[1].rect.start_col, 0);
    ASSERT_EQ(g_cb.erase[1].rect.end_col, 6);
    ASSERT_EQ(g_cb.erase[1].selective, false);

    // Cursor does not move
    ASSERT_CURSOR(state, 10, 5);
}


// ===== ED 2: Erase entire display =====
TEST(seq_ed_2_erase_all)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Position somewhere non-origin to verify cursor stays
    push(vt, "\e[5;10H");
    ASSERT_CURSOR(state, 4, 9);

    callbacks_clear();
    push(vt, "\e[2J");

    // Expect a single erase rect covering the entire display
    ASSERT_EQ(g_cb.erase_count, 1);

    ASSERT_EQ(g_cb.erase[0].rect.start_row, 0);
    ASSERT_EQ(g_cb.erase[0].rect.end_row, 25);
    ASSERT_EQ(g_cb.erase[0].rect.start_col, 0);
    ASSERT_EQ(g_cb.erase[0].rect.end_col, 80);
    ASSERT_EQ(g_cb.erase[0].selective, false);

    // Cursor does not move
    ASSERT_CURSOR(state, 4, 9);
}


// ===== ED 3: Clear scrollback buffer =====
TEST(seq_ed_3_clear_scrollback)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    push(vt, "\e[3J");

    ASSERT_EQ(g_cb.sb_clear_count, 1);
    // ED 3 should not generate any erase callbacks
    ASSERT_EQ(g_cb.erase_count, 0);
}


// ===== ED with no parameter defaults to mode 0 =====
TEST(seq_ed_default_is_0)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Position cursor at row 10, col 5
    push(vt, "\e[11;6H");
    ASSERT_CURSOR(state, 10, 5);

    callbacks_clear();
    push(vt, "\e[J"); // no param => default 0

    // Should behave identically to \e[0J
    ASSERT_EQ(g_cb.erase_count, 2);

    ASSERT_EQ(g_cb.erase[0].rect.start_row, 10);
    ASSERT_EQ(g_cb.erase[0].rect.end_row, 11);
    ASSERT_EQ(g_cb.erase[0].rect.start_col, 5);
    ASSERT_EQ(g_cb.erase[0].rect.end_col, 80);

    ASSERT_EQ(g_cb.erase[1].rect.start_row, 11);
    ASSERT_EQ(g_cb.erase[1].rect.end_row, 25);
    ASSERT_EQ(g_cb.erase[1].rect.start_col, 0);
    ASSERT_EQ(g_cb.erase[1].rect.end_col, 80);

    ASSERT_CURSOR(state, 10, 5);
}


// ===== ED 0 at origin erases entire display =====
TEST(seq_ed_at_origin)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Cursor at 0,0 (home position)
    push(vt, "\e[H");
    ASSERT_CURSOR(state, 0, 0);

    callbacks_clear();
    push(vt, "\e[0J");

        // From origin, ED 0 erases the entire display.
    // When cursor is at col 0, the "remainder of current line" rect
    // spans the full row width, so libvterm may emit a single rect
    // covering rows 0..25 cols 0..80 or two rects where the first
    // is row 0 col 0..80 and the second is rows 1..25 col 0..80.
    //
    // Based on the library behavior: at col 0, the first rect is
    // row 0..1 col 0..80, second is rows 1..25 col 0..80.
    ASSERT_EQ(g_cb.erase_count, 2);

    ASSERT_EQ(g_cb.erase[0].rect.start_row, 0);
    ASSERT_EQ(g_cb.erase[0].rect.end_row, 1);
    ASSERT_EQ(g_cb.erase[0].rect.start_col, 0);
    ASSERT_EQ(g_cb.erase[0].rect.end_col, 80);

    ASSERT_EQ(g_cb.erase[1].rect.start_row, 1);
    ASSERT_EQ(g_cb.erase[1].rect.end_row, 25);
    ASSERT_EQ(g_cb.erase[1].rect.start_col, 0);
    ASSERT_EQ(g_cb.erase[1].rect.end_col, 80);

    ASSERT_CURSOR(state, 0, 0);
}


// ===== ED 1 at bottom-right erases entire display =====
TEST(seq_ed_at_bottom_right)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Position cursor at last cell: row 24, col 79 (1-based: 25,80)
    push(vt, "\e[25;80H");
    ASSERT_CURSOR(state, 24, 79);

    callbacks_clear();
    push(vt, "\e[1J");

        // ED 1 from bottom-right erases entire display.
    // Two rects: rows above (0..24, cols 0..80) and
    // cursor row up to cursor col inclusive (24..25, cols 0..80).
    ASSERT_EQ(g_cb.erase_count, 2);

    ASSERT_EQ(g_cb.erase[0].rect.start_row, 0);
    ASSERT_EQ(g_cb.erase[0].rect.end_row, 24);
    ASSERT_EQ(g_cb.erase[0].rect.start_col, 0);
    ASSERT_EQ(g_cb.erase[0].rect.end_col, 80);

    ASSERT_EQ(g_cb.erase[1].rect.start_row, 24);
    ASSERT_EQ(g_cb.erase[1].rect.end_row, 25);
    ASSERT_EQ(g_cb.erase[1].rect.start_col, 0);
    ASSERT_EQ(g_cb.erase[1].rect.end_col, 80);

    ASSERT_CURSOR(state, 24, 79);
}


// ===== DECSED: Selective Erase in Display =====
TEST(seq_decsed_selective)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Position at row 10, col 5
    push(vt, "\e[11;6H");
    ASSERT_CURSOR(state, 10, 5);

    // DECSED 0 (erase below, selective)
    callbacks_clear();
    push(vt, "\e[?J");

    ASSERT_EQ(g_cb.erase_count, 2);

    ASSERT_EQ(g_cb.erase[0].rect.start_row, 10);
    ASSERT_EQ(g_cb.erase[0].rect.end_row, 11);
    ASSERT_EQ(g_cb.erase[0].rect.start_col, 5);
    ASSERT_EQ(g_cb.erase[0].rect.end_col, 80);
    ASSERT_EQ(g_cb.erase[0].selective, true);

    ASSERT_EQ(g_cb.erase[1].rect.start_row, 11);
    ASSERT_EQ(g_cb.erase[1].rect.end_row, 25);
    ASSERT_EQ(g_cb.erase[1].rect.start_col, 0);
    ASSERT_EQ(g_cb.erase[1].rect.end_col, 80);
    ASSERT_EQ(g_cb.erase[1].selective, true);

    ASSERT_CURSOR(state, 10, 5);

    // DECSED 1 (erase above, selective)
    callbacks_clear();
    push(vt, "\e[?1J");

    ASSERT_EQ(g_cb.erase_count, 2);

    ASSERT_EQ(g_cb.erase[0].rect.start_row, 0);
    ASSERT_EQ(g_cb.erase[0].rect.end_row, 10);
    ASSERT_EQ(g_cb.erase[0].rect.start_col, 0);
    ASSERT_EQ(g_cb.erase[0].rect.end_col, 80);
    ASSERT_EQ(g_cb.erase[0].selective, true);

    ASSERT_EQ(g_cb.erase[1].rect.start_row, 10);
    ASSERT_EQ(g_cb.erase[1].rect.end_row, 11);
    ASSERT_EQ(g_cb.erase[1].rect.start_col, 0);
    ASSERT_EQ(g_cb.erase[1].rect.end_col, 6);
    ASSERT_EQ(g_cb.erase[1].selective, true);

    ASSERT_CURSOR(state, 10, 5);

    // DECSED 2 (erase all, selective)
    callbacks_clear();
    push(vt, "\e[?2J");

    ASSERT_EQ(g_cb.erase_count, 1);

    ASSERT_EQ(g_cb.erase[0].rect.start_row, 0);
    ASSERT_EQ(g_cb.erase[0].rect.end_row, 25);
    ASSERT_EQ(g_cb.erase[0].rect.start_col, 0);
    ASSERT_EQ(g_cb.erase[0].rect.end_col, 80);
    ASSERT_EQ(g_cb.erase[0].selective, true);

    ASSERT_CURSOR(state, 10, 5);
}
