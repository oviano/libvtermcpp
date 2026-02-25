// test_68_screen_termprops.cpp — screen termprop tests
// Ported from upstream libvterm t/68screen_termprops.test

#include "harness.h"

// RESET fires default settermprop callbacks on the screen layer
TEST(screen_termprops_reset)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    Screen& screen = vt.screen();
    screen.set_callbacks(screen_cbs);
    screen.reset(true);
    callbacks_clear();

    callbacks_clear();
    screen.reset(true);

    ASSERT_EQ(g_cb.settermprop_count, 3);
    ASSERT_EQ(g_cb.settermprop[0].prop, Prop::CursorVisible);
    ASSERT_EQ(g_cb.settermprop[0].val.boolean, true);
    ASSERT_EQ(g_cb.settermprop[1].prop, Prop::CursorBlink);
    ASSERT_EQ(g_cb.settermprop[1].val.boolean, true);
    ASSERT_EQ(g_cb.settermprop[2].prop, Prop::CursorShape);
    ASSERT_EQ(g_cb.settermprop[2].val.number, static_cast<int32_t>(CursorShape::Block));
}

// Cursor visibility
TEST(screen_termprops_cursor_visible)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    Screen& screen = vt.screen();
    screen.set_callbacks(screen_cbs);
    screen.reset(true);
    callbacks_clear();

    push(vt, "\x1b[?25h");
    ASSERT_EQ(g_cb.settermprop_count, 1);
    ASSERT_EQ(g_cb.settermprop[0].prop, Prop::CursorVisible);
    ASSERT_EQ(g_cb.settermprop[0].val.boolean, true);

    callbacks_clear();

    push(vt, "\x1b[?25l");
    ASSERT_EQ(g_cb.settermprop_count, 1);
    ASSERT_EQ(g_cb.settermprop[0].prop, Prop::CursorVisible);
    ASSERT_EQ(g_cb.settermprop[0].val.boolean, false);
}

// Title
TEST(screen_termprops_title)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    Screen& screen = vt.screen();
    screen.set_callbacks(screen_cbs);
    screen.reset(true);
    callbacks_clear();

    push(vt, "\x1b]2;Here is my title\a");

    ASSERT_TRUE(g_cb.settermprop_count >= 1);
    ASSERT_EQ(g_cb.settermprop[0].prop, Prop::Title);
    ASSERT_TRUE(g_cb.settermprop[0].val.string.initial);
    ASSERT_TRUE(g_cb.settermprop[g_cb.settermprop_count - 1].val.string.final_);
}
