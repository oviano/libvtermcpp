// test_67_screen_dbl_wh.cpp — screen double-width/double-height tests
// Ported from upstream libvterm t/67screen_dbl_wh.test

#include "harness.h"

// Single Width, Single Height
TEST(screen_dbl_wh_swsh)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    Screen& screen = vt.screen();
    screen.set_callbacks(screen_cbs);
    screen.reset(true);

    push(vt, "\e#5");
    push(vt, "abcde");

    {
        Pos pos = { .row = 0, .col = 0 };
        ScreenCell cell;
        (void)screen.get_cell(pos, cell);
        ASSERT_EQ(cell.chars[0], 0x61);
        ASSERT_EQ(cell.width, 1);
        ASSERT_EQ(cell.attrs.dwl, 0);
        ASSERT_EQ(cell.attrs.dhl, 0);
        Color fg = cell.fg;
        Color bg = cell.bg;
        screen.convert_color_to_rgb(fg);
        screen.convert_color_to_rgb(bg);
        ASSERT_EQ(fg.rgb.red, 240);
        ASSERT_EQ(fg.rgb.green, 240);
        ASSERT_EQ(fg.rgb.blue, 240);
        ASSERT_EQ(bg.rgb.red, 0);
        ASSERT_EQ(bg.rgb.green, 0);
        ASSERT_EQ(bg.rgb.blue, 0);
    }
}

// Double Width, Single Height
TEST(screen_dbl_wh_dwsh)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    Screen& screen = vt.screen();
    screen.set_callbacks(screen_cbs);
    screen.reset(true);

    push(vt, "\e#6");
    push(vt, "abcde");

    {
        Pos pos = { .row = 0, .col = 0 };
        ScreenCell cell;
        (void)screen.get_cell(pos, cell);
        ASSERT_EQ(cell.chars[0], 0x61);
        ASSERT_EQ(cell.width, 1);
        ASSERT_EQ(cell.attrs.dwl, 1);
        Color fg = cell.fg;
        Color bg = cell.bg;
        screen.convert_color_to_rgb(fg);
        screen.convert_color_to_rgb(bg);
        ASSERT_EQ(fg.rgb.red, 240);
        ASSERT_EQ(fg.rgb.green, 240);
        ASSERT_EQ(fg.rgb.blue, 240);
        ASSERT_EQ(bg.rgb.red, 0);
        ASSERT_EQ(bg.rgb.green, 0);
        ASSERT_EQ(bg.rgb.blue, 0);
    }
}

// Double Height
TEST(screen_dbl_wh_dh)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    Screen& screen = vt.screen();
    screen.set_callbacks(screen_cbs);
    screen.reset(true);

    push(vt, "\e#3");
    push(vt, "abcde");
    push(vt, "\r\n\e#4");
    push(vt, "abcde");

    {
        Pos pos = { .row = 0, .col = 0 };
        ScreenCell cell;
        (void)screen.get_cell(pos, cell);
        ASSERT_EQ(cell.chars[0], 0x61);
        ASSERT_EQ(cell.width, 1);
        ASSERT_EQ(cell.attrs.dwl, 1);
        ASSERT_EQ(cell.attrs.dhl, 1);
        Color fg = cell.fg;
        Color bg = cell.bg;
        screen.convert_color_to_rgb(fg);
        screen.convert_color_to_rgb(bg);
        ASSERT_EQ(fg.rgb.red, 240);
        ASSERT_EQ(fg.rgb.green, 240);
        ASSERT_EQ(fg.rgb.blue, 240);
        ASSERT_EQ(bg.rgb.red, 0);
        ASSERT_EQ(bg.rgb.green, 0);
        ASSERT_EQ(bg.rgb.blue, 0);
    }

    {
        Pos pos = { .row = 1, .col = 0 };
        ScreenCell cell;
        (void)screen.get_cell(pos, cell);
        ASSERT_EQ(cell.chars[0], 0x61);
        ASSERT_EQ(cell.width, 1);
        ASSERT_EQ(cell.attrs.dwl, 1);
        ASSERT_EQ(cell.attrs.dhl, 2);
        Color fg = cell.fg;
        Color bg = cell.bg;
        screen.convert_color_to_rgb(fg);
        screen.convert_color_to_rgb(bg);
        ASSERT_EQ(fg.rgb.red, 240);
        ASSERT_EQ(fg.rgb.green, 240);
        ASSERT_EQ(fg.rgb.blue, 240);
        ASSERT_EQ(bg.rgb.red, 0);
        ASSERT_EQ(bg.rgb.green, 0);
        ASSERT_EQ(bg.rgb.blue, 0);
    }
}

// Late change
TEST(screen_dbl_wh_late_change)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    Screen& screen = vt.screen();
    screen.set_callbacks(screen_cbs);
    screen.reset(true);

    push(vt, "abcde");

    {
        Pos pos = { .row = 0, .col = 0 };
        ScreenCell cell;
        (void)screen.get_cell(pos, cell);
        ASSERT_EQ(cell.chars[0], 0x61);
        ASSERT_EQ(cell.width, 1);
        ASSERT_EQ(cell.attrs.dwl, 0);
        Color fg = cell.fg;
        Color bg = cell.bg;
        screen.convert_color_to_rgb(fg);
        screen.convert_color_to_rgb(bg);
        ASSERT_EQ(fg.rgb.red, 240);
        ASSERT_EQ(fg.rgb.green, 240);
        ASSERT_EQ(fg.rgb.blue, 240);
        ASSERT_EQ(bg.rgb.red, 0);
        ASSERT_EQ(bg.rgb.green, 0);
        ASSERT_EQ(bg.rgb.blue, 0);
    }

    push(vt, "\e#6");

    {
        Pos pos = { .row = 0, .col = 0 };
        ScreenCell cell;
        (void)screen.get_cell(pos, cell);
        ASSERT_EQ(cell.chars[0], 0x61);
        ASSERT_EQ(cell.width, 1);
        ASSERT_EQ(cell.attrs.dwl, 1);
        Color fg = cell.fg;
        Color bg = cell.bg;
        screen.convert_color_to_rgb(fg);
        screen.convert_color_to_rgb(bg);
        ASSERT_EQ(fg.rgb.red, 240);
        ASSERT_EQ(fg.rgb.green, 240);
        ASSERT_EQ(fg.rgb.blue, 240);
        ASSERT_EQ(bg.rgb.red, 0);
        ASSERT_EQ(bg.rgb.green, 0);
        ASSERT_EQ(bg.rgb.blue, 0);
    }
}

// DWL doesn't spill over on scroll
TEST(screen_dbl_wh_dwl_scroll)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    Screen& screen = vt.screen();
    screen.set_callbacks(screen_cbs);
    screen.reset(true);

    push(vt, "\e[25H\e#6Final\r\n");

    {
        Pos pos = { .row = 23, .col = 0 };
        ScreenCell cell;
        (void)screen.get_cell(pos, cell);
        ASSERT_EQ(cell.chars[0], 0x46);
        ASSERT_EQ(cell.width, 1);
        ASSERT_EQ(cell.attrs.dwl, 1);
        Color fg = cell.fg;
        Color bg = cell.bg;
        screen.convert_color_to_rgb(fg);
        screen.convert_color_to_rgb(bg);
        ASSERT_EQ(fg.rgb.red, 240);
        ASSERT_EQ(fg.rgb.green, 240);
        ASSERT_EQ(fg.rgb.blue, 240);
        ASSERT_EQ(bg.rgb.red, 0);
        ASSERT_EQ(bg.rgb.green, 0);
        ASSERT_EQ(bg.rgb.blue, 0);
    }

    {
        Pos pos = { .row = 24, .col = 0 };
        ScreenCell cell;
        (void)screen.get_cell(pos, cell);
        ASSERT_EQ(cell.chars[0], 0);
        ASSERT_EQ(cell.width, 1);
        ASSERT_EQ(cell.attrs.dwl, 0);
        Color fg = cell.fg;
        Color bg = cell.bg;
        screen.convert_color_to_rgb(fg);
        screen.convert_color_to_rgb(bg);
        ASSERT_EQ(fg.rgb.red, 240);
        ASSERT_EQ(fg.rgb.green, 240);
        ASSERT_EQ(fg.rgb.blue, 240);
        ASSERT_EQ(bg.rgb.red, 0);
        ASSERT_EQ(bg.rgb.green, 0);
        ASSERT_EQ(bg.rgb.blue, 0);
    }
}
