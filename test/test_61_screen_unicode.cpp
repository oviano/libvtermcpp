// test_61_screen_unicode.cpp — screen layer Unicode tests
// Ported from upstream libvterm t/61screen_unicode.test

#include "harness.h"

// Single width UTF-8
// U+00C1 = 0xC3 0x81  name: LATIN CAPITAL LETTER A WITH ACUTE
// U+00E9 = 0xC3 0xA9  name: LATIN SMALL LETTER E WITH ACUTE
TEST(screen_unicode_single_width)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    Screen& screen = vt.screen();
    screen.set_callbacks(screen_cbs);
    screen.reset(true);

    push(vt, "\xC3\x81\xC3\xA9");

    // ?screen_row 0 = 0xc1,0xe9
    {
        std::array<uint32_t, 256> chars{};
        Rect rect = { .start_row = 0, .end_row = 1, .start_col = 0, .end_col = 80 };
        size_t len = screen.get_chars(std::span{chars.data(), chars.size()}, rect);
        ASSERT_TRUE(len >= 2);
        ASSERT_EQ(chars[0], 0xc1);
        ASSERT_EQ(chars[1], 0xe9);
    }

    // ?screen_text 0,0,1,80 = 0xc3,0x81,0xc3,0xa9
    {
        std::array<char, 256> text{};
        Rect rect = { .start_row = 0, .end_row = 1, .start_col = 0, .end_col = 80 };
        size_t len = screen.get_text(std::span{text.data(), text.size()}, rect);
        ASSERT_TRUE(len >= 4);
        ASSERT_EQ(static_cast<uint8_t>(text[0]), 0xc3);
        ASSERT_EQ(static_cast<uint8_t>(text[1]), 0x81);
        ASSERT_EQ(static_cast<uint8_t>(text[2]), 0xc3);
        ASSERT_EQ(static_cast<uint8_t>(text[3]), 0xa9);
    }

    // ?screen_cell 0,0 = {0xc1} width=1 attrs={} fg=rgb(240,240,240) bg=rgb(0,0,0)
    {
        Pos pos = { .row = 0, .col = 0 };
        ScreenCell cell;
        (void)screen.get_cell(pos, cell);
        ASSERT_EQ(cell.chars[0], 0xc1);
        ASSERT_EQ(cell.width, 1);
        // attrs all default (zero)
        ASSERT_EQ(cell.attrs.bold, false);
        ASSERT_EQ(cell.attrs.underline, Underline::Off);
        ASSERT_EQ(cell.attrs.italic, false);
        ASSERT_EQ(cell.attrs.blink, false);
        ASSERT_EQ(cell.attrs.reverse, false);
        ASSERT_EQ(cell.attrs.conceal, false);
        ASSERT_EQ(cell.attrs.strike, false);
        ASSERT_EQ(cell.attrs.font, 0);
        // fg=rgb(240,240,240) bg=rgb(0,0,0)
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

// Wide char
// U+FF10 = 0xEF 0xBC 0x90  name: FULLWIDTH DIGIT ZERO
TEST(screen_unicode_wide_char)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    Screen& screen = vt.screen();
    screen.set_callbacks(screen_cbs);
    screen.reset(true);

    push(vt, "0123\e[H");
    push(vt, "\xEF\xBC\x90");

    // ?screen_row 0 = 0xff10,0x32,0x33
    {
        std::array<uint32_t, 256> chars{};
        Rect rect = { .start_row = 0, .end_row = 1, .start_col = 0, .end_col = 80 };
        size_t len = screen.get_chars(std::span{chars.data(), chars.size()}, rect);
        ASSERT_TRUE(len >= 3);
        ASSERT_EQ(chars[0], 0xff10);
        ASSERT_EQ(chars[1], 0x32);
        ASSERT_EQ(chars[2], 0x33);
    }

    // ?screen_text 0,0,1,80 = 0xef,0xbc,0x90,0x32,0x33
    {
        std::array<char, 256> text{};
        Rect rect = { .start_row = 0, .end_row = 1, .start_col = 0, .end_col = 80 };
        size_t len = screen.get_text(std::span{text.data(), text.size()}, rect);
        ASSERT_TRUE(len >= 5);
        ASSERT_EQ(static_cast<uint8_t>(text[0]), 0xef);
        ASSERT_EQ(static_cast<uint8_t>(text[1]), 0xbc);
        ASSERT_EQ(static_cast<uint8_t>(text[2]), 0x90);
        ASSERT_EQ(static_cast<uint8_t>(text[3]), 0x32);
        ASSERT_EQ(static_cast<uint8_t>(text[4]), 0x33);
    }

    // ?screen_cell 0,0 = {0xff10} width=2 attrs={} fg=rgb(240,240,240) bg=rgb(0,0,0)
    {
        Pos pos = { .row = 0, .col = 0 };
        ScreenCell cell;
        (void)screen.get_cell(pos, cell);
        ASSERT_EQ(cell.chars[0], 0xff10);
        ASSERT_EQ(cell.width, 2);
        ASSERT_EQ(cell.attrs.bold, false);
        ASSERT_EQ(cell.attrs.underline, Underline::Off);
        ASSERT_EQ(cell.attrs.italic, false);
        ASSERT_EQ(cell.attrs.blink, false);
        ASSERT_EQ(cell.attrs.reverse, false);
        ASSERT_EQ(cell.attrs.conceal, false);
        ASSERT_EQ(cell.attrs.strike, false);
        ASSERT_EQ(cell.attrs.font, 0);
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

// Combining char
// U+0301 = 0xCC 0x81  name: COMBINING ACUTE
TEST(screen_unicode_combining_char)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    Screen& screen = vt.screen();
    screen.set_callbacks(screen_cbs);
    screen.reset(true);

    push(vt, "0123\e[H");
    push(vt, "e\xCC\x81");

    // ?screen_row 0 = 0x65,0x301,0x31,0x32,0x33
    {
        std::array<uint32_t, 256> chars{};
        Rect rect = { .start_row = 0, .end_row = 1, .start_col = 0, .end_col = 80 };
        size_t len = screen.get_chars(std::span{chars.data(), chars.size()}, rect);
        ASSERT_TRUE(len >= 5);
        ASSERT_EQ(chars[0], 0x65);
        ASSERT_EQ(chars[1], 0x301);
        ASSERT_EQ(chars[2], 0x31);
        ASSERT_EQ(chars[3], 0x32);
        ASSERT_EQ(chars[4], 0x33);
    }

    // ?screen_text 0,0,1,80 = 0x65,0xcc,0x81,0x31,0x32,0x33
    {
        std::array<char, 256> text{};
        Rect rect = { .start_row = 0, .end_row = 1, .start_col = 0, .end_col = 80 };
        size_t len = screen.get_text(std::span{text.data(), text.size()}, rect);
        ASSERT_TRUE(len >= 6);
        ASSERT_EQ(static_cast<uint8_t>(text[0]), 0x65);
        ASSERT_EQ(static_cast<uint8_t>(text[1]), 0xcc);
        ASSERT_EQ(static_cast<uint8_t>(text[2]), 0x81);
        ASSERT_EQ(static_cast<uint8_t>(text[3]), 0x31);
        ASSERT_EQ(static_cast<uint8_t>(text[4]), 0x32);
        ASSERT_EQ(static_cast<uint8_t>(text[5]), 0x33);
    }

    // ?screen_cell 0,0 = {0x65,0x301} width=1 attrs={} fg=rgb(240,240,240) bg=rgb(0,0,0)
    {
        Pos pos = { .row = 0, .col = 0 };
        ScreenCell cell;
        (void)screen.get_cell(pos, cell);
        ASSERT_EQ(cell.chars[0], 0x65);
        ASSERT_EQ(cell.chars[1], 0x301);
        ASSERT_EQ(cell.width, 1);
        ASSERT_EQ(cell.attrs.bold, false);
        ASSERT_EQ(cell.attrs.underline, Underline::Off);
        ASSERT_EQ(cell.attrs.italic, false);
        ASSERT_EQ(cell.attrs.blink, false);
        ASSERT_EQ(cell.attrs.reverse, false);
        ASSERT_EQ(cell.attrs.conceal, false);
        ASSERT_EQ(cell.attrs.strike, false);
        ASSERT_EQ(cell.attrs.font, 0);
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

// 10 combining accents should not crash
TEST(screen_unicode_10_combining_no_crash)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    Screen& screen = vt.screen();
    screen.set_callbacks(screen_cbs);
    screen.reset(true);

    push(vt, "e\xCC\x81\xCC\x82\xCC\x83\xCC\x84\xCC\x85"
             "\xCC\x86\xCC\x87\xCC\x88\xCC\x89\xCC\x8A");

        // ?screen_cell 0,0 = {0x65,0x301,0x302,0x303,0x304,0x305} width=1
    // attrs={} fg=rgb(240,240,240) bg=rgb(0,0,0)
    {
        Pos pos = { .row = 0, .col = 0 };
        ScreenCell cell;
        (void)screen.get_cell(pos, cell);
        ASSERT_EQ(cell.chars[0], 0x65);
        ASSERT_EQ(cell.chars[1], 0x301);
        ASSERT_EQ(cell.chars[2], 0x302);
        ASSERT_EQ(cell.chars[3], 0x303);
        ASSERT_EQ(cell.chars[4], 0x304);
        ASSERT_EQ(cell.chars[5], 0x305);
        ASSERT_EQ(cell.width, 1);
        ASSERT_EQ(cell.attrs.bold, false);
        ASSERT_EQ(cell.attrs.underline, Underline::Off);
        ASSERT_EQ(cell.attrs.italic, false);
        ASSERT_EQ(cell.attrs.blink, false);
        ASSERT_EQ(cell.attrs.reverse, false);
        ASSERT_EQ(cell.attrs.conceal, false);
        ASSERT_EQ(cell.attrs.strike, false);
        ASSERT_EQ(cell.attrs.font, 0);
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

// 40 combining accents in two split writes of 20 should not crash
TEST(screen_unicode_40_combining_split_no_crash)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    Screen& screen = vt.screen();
    screen.set_callbacks(screen_cbs);
    screen.reset(true);

    push(vt, "e"
         "\xCC\x81" "\xCC\x81" "\xCC\x81" "\xCC\x81" "\xCC\x81"
         "\xCC\x81" "\xCC\x81" "\xCC\x81" "\xCC\x81" "\xCC\x81"
         "\xCC\x81" "\xCC\x81" "\xCC\x81" "\xCC\x81" "\xCC\x81"
         "\xCC\x81" "\xCC\x81" "\xCC\x81" "\xCC\x81" "\xCC\x81");
    push(vt, "\xCC\x81" "\xCC\x81" "\xCC\x81" "\xCC\x81" "\xCC\x81"
             "\xCC\x81" "\xCC\x81" "\xCC\x81" "\xCC\x81" "\xCC\x81"
             "\xCC\x81" "\xCC\x81" "\xCC\x81" "\xCC\x81" "\xCC\x81"
             "\xCC\x81" "\xCC\x81" "\xCC\x81" "\xCC\x81" "\xCC\x81");

        // ?screen_cell 0,0 = {0x65,0x301,0x301,0x301,0x301,0x301} width=1
    // attrs={} fg=rgb(240,240,240) bg=rgb(0,0,0)
    {
        Pos pos = { .row = 0, .col = 0 };
        ScreenCell cell;
        (void)screen.get_cell(pos, cell);
        ASSERT_EQ(cell.chars[0], 0x65);
        ASSERT_EQ(cell.chars[1], 0x301);
        ASSERT_EQ(cell.chars[2], 0x301);
        ASSERT_EQ(cell.chars[3], 0x301);
        ASSERT_EQ(cell.chars[4], 0x301);
        ASSERT_EQ(cell.chars[5], 0x301);
        ASSERT_EQ(cell.width, 1);
        ASSERT_EQ(cell.attrs.bold, false);
        ASSERT_EQ(cell.attrs.underline, Underline::Off);
        ASSERT_EQ(cell.attrs.italic, false);
        ASSERT_EQ(cell.attrs.blink, false);
        ASSERT_EQ(cell.attrs.reverse, false);
        ASSERT_EQ(cell.attrs.conceal, false);
        ASSERT_EQ(cell.attrs.strike, false);
        ASSERT_EQ(cell.attrs.font, 0);
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

// Outputing CJK doublewidth in 80th column should wraparound to next line
// and not crash
TEST(screen_unicode_cjk_col80_wraparound)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    Screen& screen = vt.screen();
    screen.set_callbacks(screen_cbs);
    screen.reset(true);

    // Move to column 80 (1-based), then output fullwidth digit zero
    push(vt, "\e[80G\xEF\xBC\x90");

    // ?screen_cell 0,79 = {} width=1 attrs={} fg=rgb(240,240,240) bg=rgb(0,0,0)
    {
        Pos pos = { .row = 0, .col = 79 };
        ScreenCell cell;
        (void)screen.get_cell(pos, cell);
        ASSERT_EQ(cell.chars[0], 0);
        ASSERT_EQ(cell.width, 1);
        ASSERT_EQ(cell.attrs.bold, false);
        ASSERT_EQ(cell.attrs.underline, Underline::Off);
        ASSERT_EQ(cell.attrs.italic, false);
        ASSERT_EQ(cell.attrs.blink, false);
        ASSERT_EQ(cell.attrs.reverse, false);
        ASSERT_EQ(cell.attrs.conceal, false);
        ASSERT_EQ(cell.attrs.strike, false);
        ASSERT_EQ(cell.attrs.font, 0);
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

    // ?screen_cell 1,0 = {0xff10} width=2 attrs={} fg=rgb(240,240,240) bg=rgb(0,0,0)
    {
        Pos pos = { .row = 1, .col = 0 };
        ScreenCell cell;
        (void)screen.get_cell(pos, cell);
        ASSERT_EQ(cell.chars[0], 0xff10);
        ASSERT_EQ(cell.width, 2);
        ASSERT_EQ(cell.attrs.bold, false);
        ASSERT_EQ(cell.attrs.underline, Underline::Off);
        ASSERT_EQ(cell.attrs.italic, false);
        ASSERT_EQ(cell.attrs.blink, false);
        ASSERT_EQ(cell.attrs.reverse, false);
        ASSERT_EQ(cell.attrs.conceal, false);
        ASSERT_EQ(cell.attrs.strike, false);
        ASSERT_EQ(cell.attrs.font, 0);
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
