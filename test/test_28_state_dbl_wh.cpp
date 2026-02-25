// test_28_state_dbl_wh.cpp — state double-width/double-height tests
// Ported from upstream libvterm t/28state_dbl_wh.test

#include "harness.h"

// Single Width, Single Height
TEST(state_dbl_wh_single_width_single_height)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);

    state.reset(true);
    callbacks_clear();

    // DECSWL
    push(vt, "\e#5");
    push(vt, "Hello");
    ASSERT_EQ(g_cb.putglyph_count, 5);
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 0x48);
    ASSERT_EQ(g_cb.putglyph[0].width, 1);
    ASSERT_EQ(g_cb.putglyph[0].row, 0);
    ASSERT_EQ(g_cb.putglyph[0].col, 0);
    ASSERT_EQ(g_cb.putglyph[1].chars[0], 0x65);
    ASSERT_EQ(g_cb.putglyph[1].width, 1);
    ASSERT_EQ(g_cb.putglyph[1].row, 0);
    ASSERT_EQ(g_cb.putglyph[1].col, 1);
    ASSERT_EQ(g_cb.putglyph[2].chars[0], 0x6c);
    ASSERT_EQ(g_cb.putglyph[2].width, 1);
    ASSERT_EQ(g_cb.putglyph[2].row, 0);
    ASSERT_EQ(g_cb.putglyph[2].col, 2);
    ASSERT_EQ(g_cb.putglyph[3].chars[0], 0x6c);
    ASSERT_EQ(g_cb.putglyph[3].width, 1);
    ASSERT_EQ(g_cb.putglyph[3].row, 0);
    ASSERT_EQ(g_cb.putglyph[3].col, 3);
    ASSERT_EQ(g_cb.putglyph[4].chars[0], 0x6f);
    ASSERT_EQ(g_cb.putglyph[4].width, 1);
    ASSERT_EQ(g_cb.putglyph[4].row, 0);
    ASSERT_EQ(g_cb.putglyph[4].col, 4);
}

// Double Width, Single Height
TEST(state_dbl_wh_double_width_single_height)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);

    state.reset(true);
    callbacks_clear();

    // DECDWL
    push(vt, "\e#6");
    push(vt, "Hello");
    ASSERT_EQ(g_cb.putglyph_count, 5);
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 0x48);
    ASSERT_EQ(g_cb.putglyph[0].width, 1);
    ASSERT_EQ(g_cb.putglyph[0].row, 0);
    ASSERT_EQ(g_cb.putglyph[0].col, 0);
    ASSERT_EQ(g_cb.putglyph[0].dwl, 1);
    ASSERT_EQ(g_cb.putglyph[1].chars[0], 0x65);
    ASSERT_EQ(g_cb.putglyph[1].width, 1);
    ASSERT_EQ(g_cb.putglyph[1].row, 0);
    ASSERT_EQ(g_cb.putglyph[1].col, 1);
    ASSERT_EQ(g_cb.putglyph[1].dwl, 1);
    ASSERT_EQ(g_cb.putglyph[2].chars[0], 0x6c);
    ASSERT_EQ(g_cb.putglyph[2].width, 1);
    ASSERT_EQ(g_cb.putglyph[2].row, 0);
    ASSERT_EQ(g_cb.putglyph[2].col, 2);
    ASSERT_EQ(g_cb.putglyph[2].dwl, 1);
    ASSERT_EQ(g_cb.putglyph[3].chars[0], 0x6c);
    ASSERT_EQ(g_cb.putglyph[3].width, 1);
    ASSERT_EQ(g_cb.putglyph[3].row, 0);
    ASSERT_EQ(g_cb.putglyph[3].col, 3);
    ASSERT_EQ(g_cb.putglyph[3].dwl, 1);
    ASSERT_EQ(g_cb.putglyph[4].chars[0], 0x6f);
    ASSERT_EQ(g_cb.putglyph[4].width, 1);
    ASSERT_EQ(g_cb.putglyph[4].row, 0);
    ASSERT_EQ(g_cb.putglyph[4].col, 4);
    ASSERT_EQ(g_cb.putglyph[4].dwl, 1);
    ASSERT_CURSOR(state, 0, 5);

    callbacks_clear();

    // Cursor at col 40 wraps at half screen width for DWL line
    push(vt, "\e[40G" "AB");
    ASSERT_EQ(g_cb.putglyph_count, 2);
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 0x41);
    ASSERT_EQ(g_cb.putglyph[0].width, 1);
    ASSERT_EQ(g_cb.putglyph[0].row, 0);
    ASSERT_EQ(g_cb.putglyph[0].col, 39);
    ASSERT_EQ(g_cb.putglyph[0].dwl, 1);
    ASSERT_EQ(g_cb.putglyph[1].chars[0], 0x42);
    ASSERT_EQ(g_cb.putglyph[1].width, 1);
    ASSERT_EQ(g_cb.putglyph[1].row, 1);
    ASSERT_EQ(g_cb.putglyph[1].col, 0);
    ASSERT_CURSOR(state, 1, 1);
}

// Double Height
TEST(state_dbl_wh_double_height)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);

    state.reset(true);
    callbacks_clear();

    // DECDHL top half
    push(vt, "\e#3");
    push(vt, "Hello");
    ASSERT_EQ(g_cb.putglyph_count, 5);
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 0x48);
    ASSERT_EQ(g_cb.putglyph[0].width, 1);
    ASSERT_EQ(g_cb.putglyph[0].row, 0);
    ASSERT_EQ(g_cb.putglyph[0].col, 0);
    ASSERT_EQ(g_cb.putglyph[0].dwl, 1);
    ASSERT_EQ(g_cb.putglyph[0].dhl, 1);
    ASSERT_EQ(g_cb.putglyph[1].chars[0], 0x65);
    ASSERT_EQ(g_cb.putglyph[1].width, 1);
    ASSERT_EQ(g_cb.putglyph[1].row, 0);
    ASSERT_EQ(g_cb.putglyph[1].col, 1);
    ASSERT_EQ(g_cb.putglyph[1].dwl, 1);
    ASSERT_EQ(g_cb.putglyph[1].dhl, 1);
    ASSERT_EQ(g_cb.putglyph[2].chars[0], 0x6c);
    ASSERT_EQ(g_cb.putglyph[2].width, 1);
    ASSERT_EQ(g_cb.putglyph[2].row, 0);
    ASSERT_EQ(g_cb.putglyph[2].col, 2);
    ASSERT_EQ(g_cb.putglyph[2].dwl, 1);
    ASSERT_EQ(g_cb.putglyph[2].dhl, 1);
    ASSERT_EQ(g_cb.putglyph[3].chars[0], 0x6c);
    ASSERT_EQ(g_cb.putglyph[3].width, 1);
    ASSERT_EQ(g_cb.putglyph[3].row, 0);
    ASSERT_EQ(g_cb.putglyph[3].col, 3);
    ASSERT_EQ(g_cb.putglyph[3].dwl, 1);
    ASSERT_EQ(g_cb.putglyph[3].dhl, 1);
    ASSERT_EQ(g_cb.putglyph[4].chars[0], 0x6f);
    ASSERT_EQ(g_cb.putglyph[4].width, 1);
    ASSERT_EQ(g_cb.putglyph[4].row, 0);
    ASSERT_EQ(g_cb.putglyph[4].col, 4);
    ASSERT_EQ(g_cb.putglyph[4].dwl, 1);
    ASSERT_EQ(g_cb.putglyph[4].dhl, 1);
    ASSERT_CURSOR(state, 0, 5);

    callbacks_clear();

    // DECDHL bottom half on next line
    push(vt, "\r\n\e#4");
    push(vt, "Hello");
    ASSERT_EQ(g_cb.putglyph_count, 5);
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 0x48);
    ASSERT_EQ(g_cb.putglyph[0].width, 1);
    ASSERT_EQ(g_cb.putglyph[0].row, 1);
    ASSERT_EQ(g_cb.putglyph[0].col, 0);
    ASSERT_EQ(g_cb.putglyph[0].dwl, 1);
    ASSERT_EQ(g_cb.putglyph[0].dhl, 2);
    ASSERT_EQ(g_cb.putglyph[1].chars[0], 0x65);
    ASSERT_EQ(g_cb.putglyph[1].width, 1);
    ASSERT_EQ(g_cb.putglyph[1].row, 1);
    ASSERT_EQ(g_cb.putglyph[1].col, 1);
    ASSERT_EQ(g_cb.putglyph[1].dwl, 1);
    ASSERT_EQ(g_cb.putglyph[1].dhl, 2);
    ASSERT_EQ(g_cb.putglyph[2].chars[0], 0x6c);
    ASSERT_EQ(g_cb.putglyph[2].width, 1);
    ASSERT_EQ(g_cb.putglyph[2].row, 1);
    ASSERT_EQ(g_cb.putglyph[2].col, 2);
    ASSERT_EQ(g_cb.putglyph[2].dwl, 1);
    ASSERT_EQ(g_cb.putglyph[2].dhl, 2);
    ASSERT_EQ(g_cb.putglyph[3].chars[0], 0x6c);
    ASSERT_EQ(g_cb.putglyph[3].width, 1);
    ASSERT_EQ(g_cb.putglyph[3].row, 1);
    ASSERT_EQ(g_cb.putglyph[3].col, 3);
    ASSERT_EQ(g_cb.putglyph[3].dwl, 1);
    ASSERT_EQ(g_cb.putglyph[3].dhl, 2);
    ASSERT_EQ(g_cb.putglyph[4].chars[0], 0x6f);
    ASSERT_EQ(g_cb.putglyph[4].width, 1);
    ASSERT_EQ(g_cb.putglyph[4].row, 1);
    ASSERT_EQ(g_cb.putglyph[4].col, 4);
    ASSERT_EQ(g_cb.putglyph[4].dwl, 1);
    ASSERT_EQ(g_cb.putglyph[4].dhl, 2);
    ASSERT_CURSOR(state, 1, 5);
}

// Double Width scrolling
TEST(state_dbl_wh_double_width_scrolling)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);

    state.reset(true);
    callbacks_clear();

    // Move to row 20, set DECDWL, type ABC
    push(vt, "\e[20H" "\e#6" "ABC");
    ASSERT_EQ(g_cb.putglyph_count, 3);
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 0x41);
    ASSERT_EQ(g_cb.putglyph[0].width, 1);
    ASSERT_EQ(g_cb.putglyph[0].row, 19);
    ASSERT_EQ(g_cb.putglyph[0].col, 0);
    ASSERT_EQ(g_cb.putglyph[0].dwl, 1);
    ASSERT_EQ(g_cb.putglyph[1].chars[0], 0x42);
    ASSERT_EQ(g_cb.putglyph[1].width, 1);
    ASSERT_EQ(g_cb.putglyph[1].row, 19);
    ASSERT_EQ(g_cb.putglyph[1].col, 1);
    ASSERT_EQ(g_cb.putglyph[1].dwl, 1);
    ASSERT_EQ(g_cb.putglyph[2].chars[0], 0x43);
    ASSERT_EQ(g_cb.putglyph[2].width, 1);
    ASSERT_EQ(g_cb.putglyph[2].row, 19);
    ASSERT_EQ(g_cb.putglyph[2].col, 2);
    ASSERT_EQ(g_cb.putglyph[2].dwl, 1);

    callbacks_clear();

    // Scroll up by going to row 25 and pressing newline
    push(vt, "\e[25H\n");

    callbacks_clear();

    // DWL attribute should persist after scroll; now row 19 (was 20)
    push(vt, "\e[19;4H" "DE");
    ASSERT_EQ(g_cb.putglyph_count, 2);
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 0x44);
    ASSERT_EQ(g_cb.putglyph[0].width, 1);
    ASSERT_EQ(g_cb.putglyph[0].row, 18);
    ASSERT_EQ(g_cb.putglyph[0].col, 3);
    ASSERT_EQ(g_cb.putglyph[0].dwl, 1);
    ASSERT_EQ(g_cb.putglyph[1].chars[0], 0x45);
    ASSERT_EQ(g_cb.putglyph[1].width, 1);
    ASSERT_EQ(g_cb.putglyph[1].row, 18);
    ASSERT_EQ(g_cb.putglyph[1].col, 4);
    ASSERT_EQ(g_cb.putglyph[1].dwl, 1);

    callbacks_clear();

    // Scroll down by going to row 1 and doing reverse index
    push(vt, "\e[H\eM");

    callbacks_clear();

    // DWL attribute should persist after scroll; now row 20 (was 19)
    push(vt, "\e[20;6H" "FG");
    ASSERT_EQ(g_cb.putglyph_count, 2);
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 0x46);
    ASSERT_EQ(g_cb.putglyph[0].width, 1);
    ASSERT_EQ(g_cb.putglyph[0].row, 19);
    ASSERT_EQ(g_cb.putglyph[0].col, 5);
    ASSERT_EQ(g_cb.putglyph[0].dwl, 1);
    ASSERT_EQ(g_cb.putglyph[1].chars[0], 0x47);
    ASSERT_EQ(g_cb.putglyph[1].width, 1);
    ASSERT_EQ(g_cb.putglyph[1].row, 19);
    ASSERT_EQ(g_cb.putglyph[1].col, 6);
    ASSERT_EQ(g_cb.putglyph[1].dwl, 1);
}
