// test_21_state_tabstops.cpp — state tabstop tests
// Ported from upstream libvterm t/21state_tabstops.test

#include "harness.h"

// Initial
TEST(state_tabstops_initial)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);

    state.reset(true);
    callbacks_clear();

    push(vt, "\tX");
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 0x58);
    ASSERT_EQ(g_cb.putglyph[0].width, 1);
    ASSERT_EQ(g_cb.putglyph[0].row, 0);
    ASSERT_EQ(g_cb.putglyph[0].col, 8);

    callbacks_clear();

    push(vt, "\tX");
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 0x58);
    ASSERT_EQ(g_cb.putglyph[0].width, 1);
    ASSERT_EQ(g_cb.putglyph[0].row, 0);
    ASSERT_EQ(g_cb.putglyph[0].col, 16);
    ASSERT_CURSOR(state, 0, 17);
}

// HTS
TEST(state_tabstops_hts)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);

    state.reset(true);
    callbacks_clear();

    // Move to column 5, set tab stop there
    push(vt, "\e[5G\eH");

    callbacks_clear();

    // Move to column 1, tab should land at column 4 (0-indexed)
    push(vt, "\e[G\tX");
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 0x58);
    ASSERT_EQ(g_cb.putglyph[0].width, 1);
    ASSERT_EQ(g_cb.putglyph[0].row, 0);
    ASSERT_EQ(g_cb.putglyph[0].col, 4);
    ASSERT_CURSOR(state, 0, 5);
}

// TBC 0
TEST(state_tabstops_tbc_0)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);

    state.reset(true);
    callbacks_clear();

    // Set tab at column 4 first (from HTS test)
    push(vt, "\e[5G\eH");

    callbacks_clear();

    // Move to column 9 (1-indexed), clear tab stop there
    push(vt, "\e[9G\e[g");

    callbacks_clear();

    // Move to column 1, tab: should skip cleared col 8 and land at col 4, then tab to col 16
    push(vt, "\e[G\tX\tX");
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 0x58);
    ASSERT_EQ(g_cb.putglyph[0].width, 1);
    ASSERT_EQ(g_cb.putglyph[0].row, 0);
    ASSERT_EQ(g_cb.putglyph[0].col, 4);
    ASSERT_EQ(g_cb.putglyph[1].chars[0], 0x58);
    ASSERT_EQ(g_cb.putglyph[1].width, 1);
    ASSERT_EQ(g_cb.putglyph[1].row, 0);
    ASSERT_EQ(g_cb.putglyph[1].col, 16);
    ASSERT_CURSOR(state, 0, 17);
}

// TBC 3
TEST(state_tabstops_tbc_3)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);

    state.reset(true);
    callbacks_clear();

    // Clear all tab stops, set one at column 50 (1-indexed), move to column 1
    push(vt, "\e[3g\e[50G\eH\e[G");
    ASSERT_CURSOR(state, 0, 0);

    callbacks_clear();

    // Tab should jump to column 49 (0-indexed)
    push(vt, "\tX");
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 0x58);
    ASSERT_EQ(g_cb.putglyph[0].width, 1);
    ASSERT_EQ(g_cb.putglyph[0].row, 0);
    ASSERT_EQ(g_cb.putglyph[0].col, 49);
    ASSERT_CURSOR(state, 0, 50);
}

// Tabstops after resize
TEST(state_tabstops_after_resize)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);

    state.reset(true);
    vt.set_size(30, 100);
    callbacks_clear();

    // Should be 100/8 = 12 tabstops
    push(vt, "\tX");
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 0x58);
    ASSERT_EQ(g_cb.putglyph[0].width, 1);
    ASSERT_EQ(g_cb.putglyph[0].row, 0);
    ASSERT_EQ(g_cb.putglyph[0].col, 8);

    callbacks_clear();

    push(vt, "\tX");
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 0x58);
    ASSERT_EQ(g_cb.putglyph[0].width, 1);
    ASSERT_EQ(g_cb.putglyph[0].row, 0);
    ASSERT_EQ(g_cb.putglyph[0].col, 16);

    callbacks_clear();

    push(vt, "\tX");
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 0x58);
    ASSERT_EQ(g_cb.putglyph[0].width, 1);
    ASSERT_EQ(g_cb.putglyph[0].row, 0);
    ASSERT_EQ(g_cb.putglyph[0].col, 24);

    callbacks_clear();

    push(vt, "\tX");
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 0x58);
    ASSERT_EQ(g_cb.putglyph[0].width, 1);
    ASSERT_EQ(g_cb.putglyph[0].row, 0);
    ASSERT_EQ(g_cb.putglyph[0].col, 32);

    callbacks_clear();

    push(vt, "\tX");
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 0x58);
    ASSERT_EQ(g_cb.putglyph[0].width, 1);
    ASSERT_EQ(g_cb.putglyph[0].row, 0);
    ASSERT_EQ(g_cb.putglyph[0].col, 40);

    callbacks_clear();

    push(vt, "\tX");
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 0x58);
    ASSERT_EQ(g_cb.putglyph[0].width, 1);
    ASSERT_EQ(g_cb.putglyph[0].row, 0);
    ASSERT_EQ(g_cb.putglyph[0].col, 48);

    callbacks_clear();

    push(vt, "\tX");
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 0x58);
    ASSERT_EQ(g_cb.putglyph[0].width, 1);
    ASSERT_EQ(g_cb.putglyph[0].row, 0);
    ASSERT_EQ(g_cb.putglyph[0].col, 56);

    callbacks_clear();

    push(vt, "\tX");
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 0x58);
    ASSERT_EQ(g_cb.putglyph[0].width, 1);
    ASSERT_EQ(g_cb.putglyph[0].row, 0);
    ASSERT_EQ(g_cb.putglyph[0].col, 64);

    callbacks_clear();

    push(vt, "\tX");
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 0x58);
    ASSERT_EQ(g_cb.putglyph[0].width, 1);
    ASSERT_EQ(g_cb.putglyph[0].row, 0);
    ASSERT_EQ(g_cb.putglyph[0].col, 72);

    callbacks_clear();

    push(vt, "\tX");
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 0x58);
    ASSERT_EQ(g_cb.putglyph[0].width, 1);
    ASSERT_EQ(g_cb.putglyph[0].row, 0);
    ASSERT_EQ(g_cb.putglyph[0].col, 80);

    callbacks_clear();

    push(vt, "\tX");
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 0x58);
    ASSERT_EQ(g_cb.putglyph[0].width, 1);
    ASSERT_EQ(g_cb.putglyph[0].row, 0);
    ASSERT_EQ(g_cb.putglyph[0].col, 88);

    callbacks_clear();

    push(vt, "\tX");
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 0x58);
    ASSERT_EQ(g_cb.putglyph[0].width, 1);
    ASSERT_EQ(g_cb.putglyph[0].row, 0);
    ASSERT_EQ(g_cb.putglyph[0].col, 96);
    ASSERT_CURSOR(state, 0, 97);
}
