// test_64_screen_pen.cpp — screen pen/attribute tests
// Ported from upstream libvterm t/64screen_pen.test

#include "harness.h"

// Plain through DECSCNM
TEST(screen_pen_attrs)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    Screen& screen = vt.screen();
    screen.set_callbacks(screen_cbs);
    screen.reset(true);
    output_init(vt);

    // Plain
    push(vt, "A");

    {
        Pos pos = { .row = 0, .col = 0 };
        ScreenCell cell;
        (void)screen.get_cell(pos, cell);
        ASSERT_EQ(cell.chars[0], 0x41);
        ASSERT_EQ(cell.width, 1);
        ASSERT_EQ(cell.attrs.bold, false);
        ASSERT_EQ(cell.attrs.underline, Underline::Off);
        ASSERT_EQ(cell.attrs.italic, false);
        ASSERT_EQ(cell.attrs.reverse, false);
        ASSERT_EQ(cell.attrs.font, 0);
        ASSERT_EQ(cell.attrs.baseline, Baseline::Normal);
        Color fg = cell.fg, bg = cell.bg;
        screen.convert_color_to_rgb(fg);
        screen.convert_color_to_rgb(bg);
        ASSERT_EQ(fg.rgb.red, 240);
        ASSERT_EQ(fg.rgb.green, 240);
        ASSERT_EQ(fg.rgb.blue, 240);
        ASSERT_EQ(bg.rgb.red, 0);
        ASSERT_EQ(bg.rgb.green, 0);
        ASSERT_EQ(bg.rgb.blue, 0);
    }

    // Bold
    push(vt, "\x1b[1mB");

    {
        Pos pos = { .row = 0, .col = 1 };
        ScreenCell cell;
        (void)screen.get_cell(pos, cell);
        ASSERT_EQ(cell.chars[0], 0x42);
        ASSERT_EQ(cell.width, 1);
        ASSERT_EQ(cell.attrs.bold, true);
        ASSERT_EQ(cell.attrs.underline, Underline::Off);
        ASSERT_EQ(cell.attrs.italic, false);
        ASSERT_EQ(cell.attrs.reverse, false);
        ASSERT_EQ(cell.attrs.font, 0);
        ASSERT_EQ(cell.attrs.baseline, Baseline::Normal);
        Color fg = cell.fg, bg = cell.bg;
        screen.convert_color_to_rgb(fg);
        screen.convert_color_to_rgb(bg);
        ASSERT_EQ(fg.rgb.red, 240);
        ASSERT_EQ(fg.rgb.green, 240);
        ASSERT_EQ(fg.rgb.blue, 240);
        ASSERT_EQ(bg.rgb.red, 0);
        ASSERT_EQ(bg.rgb.green, 0);
        ASSERT_EQ(bg.rgb.blue, 0);
    }

    // Italic
    push(vt, "\x1b[3mC");

    {
        Pos pos = { .row = 0, .col = 2 };
        ScreenCell cell;
        (void)screen.get_cell(pos, cell);
        ASSERT_EQ(cell.chars[0], 0x43);
        ASSERT_EQ(cell.width, 1);
        ASSERT_EQ(cell.attrs.bold, true);
        ASSERT_EQ(cell.attrs.underline, Underline::Off);
        ASSERT_EQ(cell.attrs.italic, true);
        ASSERT_EQ(cell.attrs.reverse, false);
        ASSERT_EQ(cell.attrs.font, 0);
        ASSERT_EQ(cell.attrs.baseline, Baseline::Normal);
        Color fg = cell.fg, bg = cell.bg;
        screen.convert_color_to_rgb(fg);
        screen.convert_color_to_rgb(bg);
        ASSERT_EQ(fg.rgb.red, 240);
        ASSERT_EQ(fg.rgb.green, 240);
        ASSERT_EQ(fg.rgb.blue, 240);
        ASSERT_EQ(bg.rgb.red, 0);
        ASSERT_EQ(bg.rgb.green, 0);
        ASSERT_EQ(bg.rgb.blue, 0);
    }

    // Underline
    push(vt, "\x1b[4mD");

    {
        Pos pos = { .row = 0, .col = 3 };
        ScreenCell cell;
        (void)screen.get_cell(pos, cell);
        ASSERT_EQ(cell.chars[0], 0x44);
        ASSERT_EQ(cell.width, 1);
        ASSERT_EQ(cell.attrs.bold, true);
        ASSERT_EQ(cell.attrs.underline, Underline::Single);
        ASSERT_EQ(cell.attrs.italic, true);
        ASSERT_EQ(cell.attrs.reverse, false);
        ASSERT_EQ(cell.attrs.font, 0);
        ASSERT_EQ(cell.attrs.baseline, Baseline::Normal);
        Color fg = cell.fg, bg = cell.bg;
        screen.convert_color_to_rgb(fg);
        screen.convert_color_to_rgb(bg);
        ASSERT_EQ(fg.rgb.red, 240);
        ASSERT_EQ(fg.rgb.green, 240);
        ASSERT_EQ(fg.rgb.blue, 240);
        ASSERT_EQ(bg.rgb.red, 0);
        ASSERT_EQ(bg.rgb.green, 0);
        ASSERT_EQ(bg.rgb.blue, 0);
    }

    // Reset
    push(vt, "\x1b[mE");

    {
        Pos pos = { .row = 0, .col = 4 };
        ScreenCell cell;
        (void)screen.get_cell(pos, cell);
        ASSERT_EQ(cell.chars[0], 0x45);
        ASSERT_EQ(cell.width, 1);
        ASSERT_EQ(cell.attrs.bold, false);
        ASSERT_EQ(cell.attrs.underline, Underline::Off);
        ASSERT_EQ(cell.attrs.italic, false);
        ASSERT_EQ(cell.attrs.reverse, false);
        ASSERT_EQ(cell.attrs.font, 0);
        ASSERT_EQ(cell.attrs.baseline, Baseline::Normal);
        Color fg = cell.fg, bg = cell.bg;
        screen.convert_color_to_rgb(fg);
        screen.convert_color_to_rgb(bg);
        ASSERT_EQ(fg.rgb.red, 240);
        ASSERT_EQ(fg.rgb.green, 240);
        ASSERT_EQ(fg.rgb.blue, 240);
        ASSERT_EQ(bg.rgb.red, 0);
        ASSERT_EQ(bg.rgb.green, 0);
        ASSERT_EQ(bg.rgb.blue, 0);
    }

    // Font
    push(vt, "\x1b[11mF\x1b[m");

    {
        Pos pos = { .row = 0, .col = 5 };
        ScreenCell cell;
        (void)screen.get_cell(pos, cell);
        ASSERT_EQ(cell.chars[0], 0x46);
        ASSERT_EQ(cell.width, 1);
        ASSERT_EQ(cell.attrs.bold, false);
        ASSERT_EQ(cell.attrs.underline, Underline::Off);
        ASSERT_EQ(cell.attrs.italic, false);
        ASSERT_EQ(cell.attrs.reverse, false);
        ASSERT_EQ(cell.attrs.font, 1);
        ASSERT_EQ(cell.attrs.baseline, Baseline::Normal);
        Color fg = cell.fg, bg = cell.bg;
        screen.convert_color_to_rgb(fg);
        screen.convert_color_to_rgb(bg);
        ASSERT_EQ(fg.rgb.red, 240);
        ASSERT_EQ(fg.rgb.green, 240);
        ASSERT_EQ(fg.rgb.blue, 240);
        ASSERT_EQ(bg.rgb.red, 0);
        ASSERT_EQ(bg.rgb.green, 0);
        ASSERT_EQ(bg.rgb.blue, 0);
    }

    // Foreground
    push(vt, "\x1b[31mG\x1b[m");

    {
        Pos pos = { .row = 0, .col = 6 };
        ScreenCell cell;
        (void)screen.get_cell(pos, cell);
        ASSERT_EQ(cell.chars[0], 0x47);
        ASSERT_EQ(cell.width, 1);
        ASSERT_EQ(cell.attrs.bold, false);
        ASSERT_EQ(cell.attrs.underline, Underline::Off);
        ASSERT_EQ(cell.attrs.italic, false);
        ASSERT_EQ(cell.attrs.reverse, false);
        ASSERT_EQ(cell.attrs.font, 0);
        ASSERT_EQ(cell.attrs.baseline, Baseline::Normal);
        Color fg = cell.fg, bg = cell.bg;
        screen.convert_color_to_rgb(fg);
        screen.convert_color_to_rgb(bg);
        ASSERT_EQ(fg.rgb.red, 224);
        ASSERT_EQ(fg.rgb.green, 0);
        ASSERT_EQ(fg.rgb.blue, 0);
        ASSERT_EQ(bg.rgb.red, 0);
        ASSERT_EQ(bg.rgb.green, 0);
        ASSERT_EQ(bg.rgb.blue, 0);
    }

    // Background
    push(vt, "\x1b[42mH\x1b[m");

    {
        Pos pos = { .row = 0, .col = 7 };
        ScreenCell cell;
        (void)screen.get_cell(pos, cell);
        ASSERT_EQ(cell.chars[0], 0x48);
        ASSERT_EQ(cell.width, 1);
        ASSERT_EQ(cell.attrs.bold, false);
        ASSERT_EQ(cell.attrs.underline, Underline::Off);
        ASSERT_EQ(cell.attrs.italic, false);
        ASSERT_EQ(cell.attrs.reverse, false);
        ASSERT_EQ(cell.attrs.font, 0);
        ASSERT_EQ(cell.attrs.baseline, Baseline::Normal);
        Color fg = cell.fg, bg = cell.bg;
        screen.convert_color_to_rgb(fg);
        screen.convert_color_to_rgb(bg);
        ASSERT_EQ(fg.rgb.red, 240);
        ASSERT_EQ(fg.rgb.green, 240);
        ASSERT_EQ(fg.rgb.blue, 240);
        ASSERT_EQ(bg.rgb.red, 0);
        ASSERT_EQ(bg.rgb.green, 224);
        ASSERT_EQ(bg.rgb.blue, 0);
    }

    // Super/subscript
    push(vt, "x\x1b[74m0\x1b[73m2\x1b[m");

    {
        Pos pos = { .row = 0, .col = 8 };
        ScreenCell cell;
        (void)screen.get_cell(pos, cell);
        ASSERT_EQ(cell.chars[0], 0x78);
        ASSERT_EQ(cell.width, 1);
        ASSERT_EQ(cell.attrs.bold, false);
        ASSERT_EQ(cell.attrs.underline, Underline::Off);
        ASSERT_EQ(cell.attrs.italic, false);
        ASSERT_EQ(cell.attrs.reverse, false);
        ASSERT_EQ(cell.attrs.font, 0);
        ASSERT_EQ(cell.attrs.baseline, Baseline::Normal);
        Color fg = cell.fg, bg = cell.bg;
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
        Pos pos = { .row = 0, .col = 9 };
        ScreenCell cell;
        (void)screen.get_cell(pos, cell);
        ASSERT_EQ(cell.chars[0], 0x30);
        ASSERT_EQ(cell.width, 1);
        ASSERT_EQ(cell.attrs.bold, false);
        ASSERT_EQ(cell.attrs.underline, Underline::Off);
        ASSERT_EQ(cell.attrs.italic, false);
        ASSERT_EQ(cell.attrs.reverse, false);
        ASSERT_EQ(cell.attrs.font, 0);
        ASSERT_EQ(cell.attrs.baseline, Baseline::Lower);
        Color fg = cell.fg, bg = cell.bg;
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
        Pos pos = { .row = 0, .col = 10 };
        ScreenCell cell;
        (void)screen.get_cell(pos, cell);
        ASSERT_EQ(cell.chars[0], 0x32);
        ASSERT_EQ(cell.width, 1);
        ASSERT_EQ(cell.attrs.bold, false);
        ASSERT_EQ(cell.attrs.underline, Underline::Off);
        ASSERT_EQ(cell.attrs.italic, false);
        ASSERT_EQ(cell.attrs.reverse, false);
        ASSERT_EQ(cell.attrs.font, 0);
        ASSERT_EQ(cell.attrs.baseline, Baseline::Raise);
        Color fg = cell.fg, bg = cell.bg;
        screen.convert_color_to_rgb(fg);
        screen.convert_color_to_rgb(bg);
        ASSERT_EQ(fg.rgb.red, 240);
        ASSERT_EQ(fg.rgb.green, 240);
        ASSERT_EQ(fg.rgb.blue, 240);
        ASSERT_EQ(bg.rgb.red, 0);
        ASSERT_EQ(bg.rgb.green, 0);
        ASSERT_EQ(bg.rgb.blue, 0);
    }

    // EL sets only colours to end of line, not other attrs
    push(vt, "\x1b[H\x1b[7;33;44m\x1b[K");

    {
        Pos pos = { .row = 0, .col = 0 };
        ScreenCell cell;
        (void)screen.get_cell(pos, cell);
        ASSERT_EQ(cell.chars[0], 0);
        ASSERT_EQ(cell.width, 1);
        ASSERT_EQ(cell.attrs.bold, false);
        ASSERT_EQ(cell.attrs.underline, Underline::Off);
        ASSERT_EQ(cell.attrs.italic, false);
        ASSERT_EQ(cell.attrs.reverse, false);
        ASSERT_EQ(cell.attrs.font, 0);
        ASSERT_EQ(cell.attrs.baseline, Baseline::Normal);
        Color fg = cell.fg, bg = cell.bg;
        screen.convert_color_to_rgb(fg);
        screen.convert_color_to_rgb(bg);
        ASSERT_EQ(fg.rgb.red, 224);
        ASSERT_EQ(fg.rgb.green, 224);
        ASSERT_EQ(fg.rgb.blue, 0);
        ASSERT_EQ(bg.rgb.red, 0);
        ASSERT_EQ(bg.rgb.green, 0);
        ASSERT_EQ(bg.rgb.blue, 224);
    }

    {
        Pos pos = { .row = 0, .col = 79 };
        ScreenCell cell;
        (void)screen.get_cell(pos, cell);
        ASSERT_EQ(cell.chars[0], 0);
        ASSERT_EQ(cell.width, 1);
        ASSERT_EQ(cell.attrs.bold, false);
        ASSERT_EQ(cell.attrs.underline, Underline::Off);
        ASSERT_EQ(cell.attrs.italic, false);
        ASSERT_EQ(cell.attrs.reverse, false);
        ASSERT_EQ(cell.attrs.font, 0);
        ASSERT_EQ(cell.attrs.baseline, Baseline::Normal);
        Color fg = cell.fg, bg = cell.bg;
        screen.convert_color_to_rgb(fg);
        screen.convert_color_to_rgb(bg);
        ASSERT_EQ(fg.rgb.red, 224);
        ASSERT_EQ(fg.rgb.green, 224);
        ASSERT_EQ(fg.rgb.blue, 0);
        ASSERT_EQ(bg.rgb.red, 0);
        ASSERT_EQ(bg.rgb.green, 0);
        ASSERT_EQ(bg.rgb.blue, 224);
    }

    // DECSCNM
    push(vt, "R\x1b[?5h");

    {
        Pos pos = { .row = 0, .col = 0 };
        ScreenCell cell;
        (void)screen.get_cell(pos, cell);
        ASSERT_EQ(cell.chars[0], 0x52);
        ASSERT_EQ(cell.width, 1);
        ASSERT_EQ(cell.attrs.bold, false);
        ASSERT_EQ(cell.attrs.underline, Underline::Off);
        ASSERT_EQ(cell.attrs.italic, false);
        ASSERT_EQ(cell.attrs.reverse, false);
        ASSERT_EQ(cell.attrs.font, 0);
        ASSERT_EQ(cell.attrs.baseline, Baseline::Normal);
        Color fg = cell.fg, bg = cell.bg;
        screen.convert_color_to_rgb(fg);
        screen.convert_color_to_rgb(bg);
        ASSERT_EQ(fg.rgb.red, 224);
        ASSERT_EQ(fg.rgb.green, 224);
        ASSERT_EQ(fg.rgb.blue, 0);
        ASSERT_EQ(bg.rgb.red, 0);
        ASSERT_EQ(bg.rgb.green, 0);
        ASSERT_EQ(bg.rgb.blue, 224);
    }

    {
        Pos pos = { .row = 1, .col = 0 };
        ScreenCell cell;
        (void)screen.get_cell(pos, cell);
        ASSERT_EQ(cell.chars[0], 0);
        ASSERT_EQ(cell.width, 1);
        ASSERT_EQ(cell.attrs.bold, false);
        ASSERT_EQ(cell.attrs.underline, Underline::Off);
        ASSERT_EQ(cell.attrs.italic, false);
        ASSERT_EQ(cell.attrs.reverse, true);
        ASSERT_EQ(cell.attrs.font, 0);
        ASSERT_EQ(cell.attrs.baseline, Baseline::Normal);
        Color fg = cell.fg, bg = cell.bg;
        screen.convert_color_to_rgb(fg);
        screen.convert_color_to_rgb(bg);
        ASSERT_EQ(fg.rgb.red, 240);
        ASSERT_EQ(fg.rgb.green, 240);
        ASSERT_EQ(fg.rgb.blue, 240);
        ASSERT_EQ(bg.rgb.red, 0);
        ASSERT_EQ(bg.rgb.green, 0);
        ASSERT_EQ(bg.rgb.blue, 0);
    }

    output_clear();
    push(vt, "\x1b[?5$p");
    {
        constexpr std::string_view expected = "\x1b[?5;1$y";
        ASSERT_OUTPUT_BYTES(expected.data(), expected.size());
    }

    push(vt, "\x1b[?5l");

    {
        Pos pos = { .row = 0, .col = 0 };
        ScreenCell cell;
        (void)screen.get_cell(pos, cell);
        ASSERT_EQ(cell.chars[0], 0x52);
        ASSERT_EQ(cell.width, 1);
        ASSERT_EQ(cell.attrs.bold, false);
        ASSERT_EQ(cell.attrs.underline, Underline::Off);
        ASSERT_EQ(cell.attrs.italic, false);
        ASSERT_EQ(cell.attrs.reverse, true);
        ASSERT_EQ(cell.attrs.font, 0);
        ASSERT_EQ(cell.attrs.baseline, Baseline::Normal);
        Color fg = cell.fg, bg = cell.bg;
        screen.convert_color_to_rgb(fg);
        screen.convert_color_to_rgb(bg);
        ASSERT_EQ(fg.rgb.red, 224);
        ASSERT_EQ(fg.rgb.green, 224);
        ASSERT_EQ(fg.rgb.blue, 0);
        ASSERT_EQ(bg.rgb.red, 0);
        ASSERT_EQ(bg.rgb.green, 0);
        ASSERT_EQ(bg.rgb.blue, 224);
    }

    {
        Pos pos = { .row = 1, .col = 0 };
        ScreenCell cell;
        (void)screen.get_cell(pos, cell);
        ASSERT_EQ(cell.chars[0], 0);
        ASSERT_EQ(cell.width, 1);
        ASSERT_EQ(cell.attrs.bold, false);
        ASSERT_EQ(cell.attrs.underline, Underline::Off);
        ASSERT_EQ(cell.attrs.italic, false);
        ASSERT_EQ(cell.attrs.reverse, false);
        ASSERT_EQ(cell.attrs.font, 0);
        ASSERT_EQ(cell.attrs.baseline, Baseline::Normal);
        Color fg = cell.fg, bg = cell.bg;
        screen.convert_color_to_rgb(fg);
        screen.convert_color_to_rgb(bg);
        ASSERT_EQ(fg.rgb.red, 240);
        ASSERT_EQ(fg.rgb.green, 240);
        ASSERT_EQ(fg.rgb.blue, 240);
        ASSERT_EQ(bg.rgb.red, 0);
        ASSERT_EQ(bg.rgb.green, 0);
        ASSERT_EQ(bg.rgb.blue, 0);
    }

    output_clear();
    push(vt, "\x1b[?5$p");
    {
        constexpr std::string_view expected = "\x1b[?5;2$y";
        ASSERT_OUTPUT_BYTES(expected.data(), expected.size());
    }
}

// Set default colours
TEST(screen_pen_default_colors)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    Screen& screen = vt.screen();
    screen.set_callbacks(screen_cbs);
    screen.reset(true);

    push(vt, "ABC\x1b[31mDEF\x1b[m");

    {
        Pos pos = { .row = 0, .col = 0 };
        ScreenCell cell;
        (void)screen.get_cell(pos, cell);
        ASSERT_EQ(cell.chars[0], 0x41);
        ASSERT_EQ(cell.width, 1);
        Color fg = cell.fg, bg = cell.bg;
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
        Pos pos = { .row = 0, .col = 3 };
        ScreenCell cell;
        (void)screen.get_cell(pos, cell);
        ASSERT_EQ(cell.chars[0], 0x44);
        Color fg = cell.fg, bg = cell.bg;
        screen.convert_color_to_rgb(fg);
        screen.convert_color_to_rgb(bg);
        ASSERT_EQ(fg.rgb.red, 224);
        ASSERT_EQ(fg.rgb.green, 0);
        ASSERT_EQ(fg.rgb.blue, 0);
        ASSERT_EQ(bg.rgb.red, 0);
        ASSERT_EQ(bg.rgb.green, 0);
        ASSERT_EQ(bg.rgb.blue, 0);
    }

    // SETDEFAULTCOL rgb(252,253,254) — set default fg only
    {
        Color col = Color::from_rgb(252, 253, 254);
        Color bgcol = Color::from_rgb(0, 0, 0);
        screen.set_default_colors(col, bgcol);
    }

    {
        Pos pos = { .row = 0, .col = 0 };
        ScreenCell cell;
        (void)screen.get_cell(pos, cell);
        ASSERT_EQ(cell.chars[0], 0x41);
        Color fg = cell.fg, bg = cell.bg;
        screen.convert_color_to_rgb(fg);
        screen.convert_color_to_rgb(bg);
        ASSERT_EQ(fg.rgb.red, 252);
        ASSERT_EQ(fg.rgb.green, 253);
        ASSERT_EQ(fg.rgb.blue, 254);
        ASSERT_EQ(bg.rgb.red, 0);
        ASSERT_EQ(bg.rgb.green, 0);
        ASSERT_EQ(bg.rgb.blue, 0);
    }

    {
        Pos pos = { .row = 0, .col = 3 };
        ScreenCell cell;
        (void)screen.get_cell(pos, cell);
        ASSERT_EQ(cell.chars[0], 0x44);
        Color fg = cell.fg, bg = cell.bg;
        screen.convert_color_to_rgb(fg);
        screen.convert_color_to_rgb(bg);
        ASSERT_EQ(fg.rgb.red, 224);
        ASSERT_EQ(fg.rgb.green, 0);
        ASSERT_EQ(fg.rgb.blue, 0);
        ASSERT_EQ(bg.rgb.red, 0);
        ASSERT_EQ(bg.rgb.green, 0);
        ASSERT_EQ(bg.rgb.blue, 0);
    }

    // SETDEFAULTCOL rgb(250,250,250) rgb(10,20,30) — set both fg and bg
    {
        Color fgcol = Color::from_rgb(250, 250, 250);
        Color bgcol = Color::from_rgb(10, 20, 30);
        screen.set_default_colors(fgcol, bgcol);
    }

    {
        Pos pos = { .row = 0, .col = 0 };
        ScreenCell cell;
        (void)screen.get_cell(pos, cell);
        ASSERT_EQ(cell.chars[0], 0x41);
        Color fg = cell.fg, bg = cell.bg;
        screen.convert_color_to_rgb(fg);
        screen.convert_color_to_rgb(bg);
        ASSERT_EQ(fg.rgb.red, 250);
        ASSERT_EQ(fg.rgb.green, 250);
        ASSERT_EQ(fg.rgb.blue, 250);
        ASSERT_EQ(bg.rgb.red, 10);
        ASSERT_EQ(bg.rgb.green, 20);
        ASSERT_EQ(bg.rgb.blue, 30);
    }

    {
        Pos pos = { .row = 0, .col = 3 };
        ScreenCell cell;
        (void)screen.get_cell(pos, cell);
        ASSERT_EQ(cell.chars[0], 0x44);
        Color fg = cell.fg, bg = cell.bg;
        screen.convert_color_to_rgb(fg);
        screen.convert_color_to_rgb(bg);
        ASSERT_EQ(fg.rgb.red, 224);
        ASSERT_EQ(fg.rgb.green, 0);
        ASSERT_EQ(fg.rgb.blue, 0);
        ASSERT_EQ(bg.rgb.red, 10);
        ASSERT_EQ(bg.rgb.green, 20);
        ASSERT_EQ(bg.rgb.blue, 30);
    }
}
