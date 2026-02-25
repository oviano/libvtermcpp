// test_16_state_resize.cpp — state resize tests
// Ported from upstream libvterm t/16state_resize.test

#include "harness.h"

// Placement
TEST(state_resize_placement)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);

    state.reset(true);
    callbacks_clear();

    push(vt, "AB\e[79GCDE");
    ASSERT_EQ(g_cb.putglyph_count, 5);
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 0x41);
    ASSERT_EQ(g_cb.putglyph[0].width, 1);
    ASSERT_EQ(g_cb.putglyph[0].row, 0);
    ASSERT_EQ(g_cb.putglyph[0].col, 0);
    ASSERT_EQ(g_cb.putglyph[1].chars[0], 0x42);
    ASSERT_EQ(g_cb.putglyph[1].width, 1);
    ASSERT_EQ(g_cb.putglyph[1].row, 0);
    ASSERT_EQ(g_cb.putglyph[1].col, 1);
    ASSERT_EQ(g_cb.putglyph[2].chars[0], 0x43);
    ASSERT_EQ(g_cb.putglyph[2].width, 1);
    ASSERT_EQ(g_cb.putglyph[2].row, 0);
    ASSERT_EQ(g_cb.putglyph[2].col, 78);
    ASSERT_EQ(g_cb.putglyph[3].chars[0], 0x44);
    ASSERT_EQ(g_cb.putglyph[3].width, 1);
    ASSERT_EQ(g_cb.putglyph[3].row, 0);
    ASSERT_EQ(g_cb.putglyph[3].col, 79);
    ASSERT_EQ(g_cb.putglyph[4].chars[0], 0x45);
    ASSERT_EQ(g_cb.putglyph[4].width, 1);
    ASSERT_EQ(g_cb.putglyph[4].row, 1);
    ASSERT_EQ(g_cb.putglyph[4].col, 0);
}

// Resize
TEST(state_resize_resize)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);

    state.reset(true);
    callbacks_clear();

    vt.set_size(27, 85);
    push(vt, "AB\e[79GCDE");
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 0x41);
    ASSERT_EQ(g_cb.putglyph[0].width, 1);
    ASSERT_EQ(g_cb.putglyph[0].row, 0);
    ASSERT_EQ(g_cb.putglyph[0].col, 0);
    ASSERT_EQ(g_cb.putglyph[1].chars[0], 0x42);
    ASSERT_EQ(g_cb.putglyph[1].width, 1);
    ASSERT_EQ(g_cb.putglyph[1].row, 0);
    ASSERT_EQ(g_cb.putglyph[1].col, 1);
    ASSERT_EQ(g_cb.putglyph[2].chars[0], 0x43);
    ASSERT_EQ(g_cb.putglyph[2].width, 1);
    ASSERT_EQ(g_cb.putglyph[2].row, 0);
    ASSERT_EQ(g_cb.putglyph[2].col, 78);
    ASSERT_EQ(g_cb.putglyph[3].chars[0], 0x44);
    ASSERT_EQ(g_cb.putglyph[3].width, 1);
    ASSERT_EQ(g_cb.putglyph[3].row, 0);
    ASSERT_EQ(g_cb.putglyph[3].col, 79);
    ASSERT_EQ(g_cb.putglyph[4].chars[0], 0x45);
    ASSERT_EQ(g_cb.putglyph[4].width, 1);
    ASSERT_EQ(g_cb.putglyph[4].row, 0);
    ASSERT_EQ(g_cb.putglyph[4].col, 80);
    ASSERT_CURSOR(state, 0, 81);
}

// Resize without reset
TEST(state_resize_without_reset)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);

    state.reset(true);
    callbacks_clear();

    vt.set_size(27, 85);
    push(vt, "AB\e[79GCDE");
    callbacks_clear();

    vt.set_size(28, 90);
    ASSERT_CURSOR(state, 0, 81);

    push(vt, "FGHI");
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 0x46);
    ASSERT_EQ(g_cb.putglyph[0].width, 1);
    ASSERT_EQ(g_cb.putglyph[0].row, 0);
    ASSERT_EQ(g_cb.putglyph[0].col, 81);
    ASSERT_EQ(g_cb.putglyph[1].chars[0], 0x47);
    ASSERT_EQ(g_cb.putglyph[1].width, 1);
    ASSERT_EQ(g_cb.putglyph[1].row, 0);
    ASSERT_EQ(g_cb.putglyph[1].col, 82);
    ASSERT_EQ(g_cb.putglyph[2].chars[0], 0x48);
    ASSERT_EQ(g_cb.putglyph[2].width, 1);
    ASSERT_EQ(g_cb.putglyph[2].row, 0);
    ASSERT_EQ(g_cb.putglyph[2].col, 83);
    ASSERT_EQ(g_cb.putglyph[3].chars[0], 0x49);
    ASSERT_EQ(g_cb.putglyph[3].width, 1);
    ASSERT_EQ(g_cb.putglyph[3].row, 0);
    ASSERT_EQ(g_cb.putglyph[3].col, 84);
    ASSERT_CURSOR(state, 0, 85);
}

// Resize shrink moves cursor
TEST(state_resize_shrink_moves_cursor)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);

    state.reset(true);
    callbacks_clear();

    // Set up: resize to 28,90 with cursor at col 85
    vt.set_size(27, 85);
    push(vt, "AB\e[79GCDE");
    vt.set_size(28, 90);
    push(vt, "FGHI");
    callbacks_clear();

    vt.set_size(25, 80);
    ASSERT_CURSOR(state, 0, 79);
}

// Resize grow doesn't cancel phantom
TEST(state_resize_grow_no_cancel_phantom)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);

    state.reset(true);
    callbacks_clear();

    push(vt, "\e[79GAB");
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 0x41);
    ASSERT_EQ(g_cb.putglyph[0].width, 1);
    ASSERT_EQ(g_cb.putglyph[0].row, 0);
    ASSERT_EQ(g_cb.putglyph[0].col, 78);
    ASSERT_EQ(g_cb.putglyph[1].chars[0], 0x42);
    ASSERT_EQ(g_cb.putglyph[1].width, 1);
    ASSERT_EQ(g_cb.putglyph[1].row, 0);
    ASSERT_EQ(g_cb.putglyph[1].col, 79);
    ASSERT_CURSOR(state, 0, 79);

    callbacks_clear();

    vt.set_size(30, 100);
    ASSERT_CURSOR(state, 0, 80);

    push(vt, "C");
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 0x43);
    ASSERT_EQ(g_cb.putglyph[0].width, 1);
    ASSERT_EQ(g_cb.putglyph[0].row, 0);
    ASSERT_EQ(g_cb.putglyph[0].col, 80);
    ASSERT_CURSOR(state, 0, 81);
}
