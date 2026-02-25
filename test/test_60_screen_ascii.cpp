// test_60_screen_ascii.cpp — screen ASCII tests
// Ported from upstream libvterm t/60screen_ascii.test

#include "harness.h"

// Get
TEST(screen_ascii_get)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    Screen& screen = vt.screen();
    screen.enable_altscreen(true);
    screen.set_callbacks(screen_cbs);
    screen.reset(true);
    callbacks_clear();

    push(vt, "ABC");

    // movecursor 0,3
    ASSERT_EQ(g_cb.movecursor_count, 1);
    ASSERT_EQ(g_cb.movecursor[0].pos.row, 0);
    ASSERT_EQ(g_cb.movecursor[0].pos.col, 3);

    // ?screen_chars 0,0,1,3 = "ABC"
    {
        std::array<uint32_t, 256> chars{};
        Rect rect = { .start_row = 0, .end_row = 1, .start_col = 0, .end_col = 3 };
        size_t len = screen.get_chars(std::span{chars.data(), chars.size()}, rect);
        ASSERT_EQ(len, 3);
        ASSERT_EQ(chars[0], 'A');
        ASSERT_EQ(chars[1], 'B');
        ASSERT_EQ(chars[2], 'C');
    }

    // ?screen_chars 0,0,1,80 = "ABC"
    {
        std::array<uint32_t, 256> chars{};
        Rect rect = { .start_row = 0, .end_row = 1, .start_col = 0, .end_col = 80 };
        size_t len = screen.get_chars(std::span{chars.data(), chars.size()}, rect);
        ASSERT_EQ(len, 3);
        ASSERT_EQ(chars[0], 'A');
        ASSERT_EQ(chars[1], 'B');
        ASSERT_EQ(chars[2], 'C');
    }

    // ?screen_text 0,0,1,3 = 0x41,0x42,0x43
    {
        std::array<char, 256> text{};
        Rect rect = { .start_row = 0, .end_row = 1, .start_col = 0, .end_col = 3 };
        size_t len = screen.get_text(std::span{text.data(), text.size()}, rect);
        ASSERT_EQ(len, 3);
        ASSERT_EQ(static_cast<uint8_t>(text[0]), 0x41);
        ASSERT_EQ(static_cast<uint8_t>(text[1]), 0x42);
        ASSERT_EQ(static_cast<uint8_t>(text[2]), 0x43);
    }

    // ?screen_text 0,0,1,80 = 0x41,0x42,0x43
    {
        std::array<char, 256> text{};
        Rect rect = { .start_row = 0, .end_row = 1, .start_col = 0, .end_col = 80 };
        size_t len = screen.get_text(std::span{text.data(), text.size()}, rect);
        ASSERT_EQ(len, 3);
        ASSERT_EQ(static_cast<uint8_t>(text[0]), 0x41);
        ASSERT_EQ(static_cast<uint8_t>(text[1]), 0x42);
        ASSERT_EQ(static_cast<uint8_t>(text[2]), 0x43);
    }

    // ?screen_cell 0,0 = {0x41} width=1 attrs={} fg=rgb(240,240,240) bg=rgb(0,0,0)
    {
        Pos pos = { .row = 0, .col = 0 };
        ScreenCell cell;
        (void)screen.get_cell(pos, cell);
        ASSERT_EQ(cell.chars[0], 0x41);
        ASSERT_EQ(cell.width, 1);
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

    // ?screen_cell 0,1 = {0x42} width=1 attrs={} fg=rgb(240,240,240) bg=rgb(0,0,0)
    {
        Pos pos = { .row = 0, .col = 1 };
        ScreenCell cell;
        (void)screen.get_cell(pos, cell);
        ASSERT_EQ(cell.chars[0], 0x42);
        ASSERT_EQ(cell.width, 1);
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

    // ?screen_cell 0,2 = {0x43} width=1 attrs={} fg=rgb(240,240,240) bg=rgb(0,0,0)
    {
        Pos pos = { .row = 0, .col = 2 };
        ScreenCell cell;
        (void)screen.get_cell(pos, cell);
        ASSERT_EQ(cell.chars[0], 0x43);
        ASSERT_EQ(cell.width, 1);
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

    // ?screen_row 0 = "ABC"
    ASSERT_SCREEN_ROW(vt, screen, 0, "ABC");

    // ?screen_eol 0,0 = 0
    {
        Pos pos = { .row = 0, .col = 0 };
        ASSERT_EQ(screen.is_eol(pos), false);
    }

    // ?screen_eol 0,2 = 0
    {
        Pos pos = { .row = 0, .col = 2 };
        ASSERT_EQ(screen.is_eol(pos), false);
    }

    // ?screen_eol 0,3 = 1
    {
        Pos pos = { .row = 0, .col = 3 };
        ASSERT_EQ(screen.is_eol(pos), true);
    }

    callbacks_clear();

    // PUSH "\e[H" — cursor home
    push(vt, "\e[H");

    // movecursor 0,0
    ASSERT_EQ(g_cb.movecursor_count, 1);
    ASSERT_EQ(g_cb.movecursor[0].pos.row, 0);
    ASSERT_EQ(g_cb.movecursor[0].pos.col, 0);

    // ?screen_row 0 = "ABC"
    ASSERT_SCREEN_ROW(vt, screen, 0, "ABC");

    // ?screen_text 0,0,1,80 = 0x41,0x42,0x43
    {
        std::array<char, 256> text{};
        Rect rect = { .start_row = 0, .end_row = 1, .start_col = 0, .end_col = 80 };
        size_t len = screen.get_text(std::span{text.data(), text.size()}, rect);
        ASSERT_EQ(len, 3);
        ASSERT_EQ(static_cast<uint8_t>(text[0]), 0x41);
        ASSERT_EQ(static_cast<uint8_t>(text[1]), 0x42);
        ASSERT_EQ(static_cast<uint8_t>(text[2]), 0x43);
    }

    callbacks_clear();

    // PUSH "E" — overwrite first char
    push(vt, "E");

    // movecursor 0,1
    ASSERT_EQ(g_cb.movecursor_count, 1);
    ASSERT_EQ(g_cb.movecursor[0].pos.row, 0);
    ASSERT_EQ(g_cb.movecursor[0].pos.col, 1);

    // ?screen_row 0 = "EBC"
    ASSERT_SCREEN_ROW(vt, screen, 0, "EBC");

    // ?screen_text 0,0,1,80 = 0x45,0x42,0x43
    {
        std::array<char, 256> text{};
        Rect rect = { .start_row = 0, .end_row = 1, .start_col = 0, .end_col = 80 };
        size_t len = screen.get_text(std::span{text.data(), text.size()}, rect);
        ASSERT_EQ(len, 3);
        ASSERT_EQ(static_cast<uint8_t>(text[0]), 0x45);
        ASSERT_EQ(static_cast<uint8_t>(text[1]), 0x42);
        ASSERT_EQ(static_cast<uint8_t>(text[2]), 0x43);
    }
}

// Erase
TEST(screen_ascii_erase)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    Screen& screen = vt.screen();
    screen.enable_altscreen(true);
    screen.set_callbacks(screen_cbs);
    screen.reset(true);

    push(vt, "ABCDE\e[H\e[K");

    // ?screen_row 0 = ""
    ASSERT_SCREEN_ROW(vt, screen, 0, "");

    // ?screen_text 0,0,1,80 = (empty)
    {
        std::array<char, 256> text{};
        Rect rect = { .start_row = 0, .end_row = 1, .start_col = 0, .end_col = 80 };
        size_t len = screen.get_text(std::span{text.data(), text.size()}, rect);
        ASSERT_EQ(len, 0);
    }
}

// Copycell — insert
TEST(screen_ascii_copycell_insert)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    Screen& screen = vt.screen();
    screen.enable_altscreen(true);
    screen.set_callbacks(screen_cbs);
    screen.reset(true);

    push(vt, "ABC\e[H\e[@");
    push(vt, "1");

    // ?screen_row 0 = "1ABC"
    ASSERT_SCREEN_ROW(vt, screen, 0, "1ABC");
}

// Copycell — delete
TEST(screen_ascii_copycell_delete)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    Screen& screen = vt.screen();
    screen.enable_altscreen(true);
    screen.set_callbacks(screen_cbs);
    screen.reset(true);

    push(vt, "ABC\e[H\e[P");

    // ?screen_chars 0,0,1,1 = "B"
    {
        std::array<uint32_t, 256> chars{};
        Rect rect = { .start_row = 0, .end_row = 1, .start_col = 0, .end_col = 1 };
        size_t len = screen.get_chars(std::span{chars.data(), chars.size()}, rect);
        ASSERT_EQ(len, 1);
        ASSERT_EQ(chars[0], 'B');
    }

    // ?screen_chars 0,1,1,2 = "C"
    {
        std::array<uint32_t, 256> chars{};
        Rect rect = { .start_row = 0, .end_row = 1, .start_col = 1, .end_col = 2 };
        size_t len = screen.get_chars(std::span{chars.data(), chars.size()}, rect);
        ASSERT_EQ(len, 1);
        ASSERT_EQ(chars[0], 'C');
    }

    // ?screen_chars 0,0,1,80 = "BC"
    {
        std::array<uint32_t, 256> chars{};
        Rect rect = { .start_row = 0, .end_row = 1, .start_col = 0, .end_col = 80 };
        size_t len = screen.get_chars(std::span{chars.data(), chars.size()}, rect);
        ASSERT_EQ(len, 2);
        ASSERT_EQ(chars[0], 'B');
        ASSERT_EQ(chars[1], 'C');
    }
}

// Space padding
TEST(screen_ascii_space_padding)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    Screen& screen = vt.screen();
    screen.enable_altscreen(true);
    screen.set_callbacks(screen_cbs);
    screen.reset(true);

    push(vt, "Hello\e[CWorld");

    // ?screen_row 0 = "Hello World"
    ASSERT_SCREEN_ROW(vt, screen, 0, "Hello World");

    // ?screen_text 0,0,1,80 = 0x48,0x65,0x6c,0x6c,0x6f,0x20,0x57,0x6f,0x72,0x6c,0x64
    {
        std::array<char, 256> text{};
        Rect rect = { .start_row = 0, .end_row = 1, .start_col = 0, .end_col = 80 };
        size_t len = screen.get_text(std::span{text.data(), text.size()}, rect);
        ASSERT_EQ(len, 11);
        ASSERT_EQ(static_cast<uint8_t>(text[0]), 0x48);
        ASSERT_EQ(static_cast<uint8_t>(text[1]), 0x65);
        ASSERT_EQ(static_cast<uint8_t>(text[2]), 0x6c);
        ASSERT_EQ(static_cast<uint8_t>(text[3]), 0x6c);
        ASSERT_EQ(static_cast<uint8_t>(text[4]), 0x6f);
        ASSERT_EQ(static_cast<uint8_t>(text[5]), 0x20);
        ASSERT_EQ(static_cast<uint8_t>(text[6]), 0x57);
        ASSERT_EQ(static_cast<uint8_t>(text[7]), 0x6f);
        ASSERT_EQ(static_cast<uint8_t>(text[8]), 0x72);
        ASSERT_EQ(static_cast<uint8_t>(text[9]), 0x6c);
        ASSERT_EQ(static_cast<uint8_t>(text[10]), 0x64);
    }
}

// Linefeed padding
TEST(screen_ascii_linefeed_padding)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    Screen& screen = vt.screen();
    screen.enable_altscreen(true);
    screen.set_callbacks(screen_cbs);
    screen.reset(true);

    push(vt, "Hello\r\nWorld");

    // ?screen_chars 0,0,2,80 = "Hello\nWorld"
    {
        std::array<uint32_t, 256> chars{};
        Rect rect = { .start_row = 0, .end_row = 2, .start_col = 0, .end_col = 80 };
        size_t len = screen.get_chars(std::span{chars.data(), chars.size()}, rect);
        ASSERT_EQ(len, 11);
        ASSERT_EQ(chars[0], 'H');
        ASSERT_EQ(chars[1], 'e');
        ASSERT_EQ(chars[2], 'l');
        ASSERT_EQ(chars[3], 'l');
        ASSERT_EQ(chars[4], 'o');
        ASSERT_EQ(chars[5], '\n');
        ASSERT_EQ(chars[6], 'W');
        ASSERT_EQ(chars[7], 'o');
        ASSERT_EQ(chars[8], 'r');
        ASSERT_EQ(chars[9], 'l');
        ASSERT_EQ(chars[10], 'd');
    }

    // ?screen_text 0,0,2,80 = 0x48,0x65,0x6c,0x6c,0x6f,0x0a,0x57,0x6f,0x72,0x6c,0x64
    {
        std::array<char, 256> text{};
        Rect rect = { .start_row = 0, .end_row = 2, .start_col = 0, .end_col = 80 };
        size_t len = screen.get_text(std::span{text.data(), text.size()}, rect);
        ASSERT_EQ(len, 11);
        ASSERT_EQ(static_cast<uint8_t>(text[0]), 0x48);
        ASSERT_EQ(static_cast<uint8_t>(text[1]), 0x65);
        ASSERT_EQ(static_cast<uint8_t>(text[2]), 0x6c);
        ASSERT_EQ(static_cast<uint8_t>(text[3]), 0x6c);
        ASSERT_EQ(static_cast<uint8_t>(text[4]), 0x6f);
        ASSERT_EQ(static_cast<uint8_t>(text[5]), 0x0a);
        ASSERT_EQ(static_cast<uint8_t>(text[6]), 0x57);
        ASSERT_EQ(static_cast<uint8_t>(text[7]), 0x6f);
        ASSERT_EQ(static_cast<uint8_t>(text[8]), 0x72);
        ASSERT_EQ(static_cast<uint8_t>(text[9]), 0x6c);
        ASSERT_EQ(static_cast<uint8_t>(text[10]), 0x64);
    }
}

// Altscreen
TEST(screen_ascii_altscreen)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    Screen& screen = vt.screen();
    screen.enable_altscreen(true);
    screen.set_callbacks(screen_cbs);
    screen.reset(true);

    push(vt, "P");

    // ?screen_row 0 = "P"
    ASSERT_SCREEN_ROW(vt, screen, 0, "P");

    // PUSH "\e[?1049h" — switch to altscreen
    push(vt, "\e[?1049h");

    // ?screen_row 0 = ""
    ASSERT_SCREEN_ROW(vt, screen, 0, "");

    // PUSH "\e[2K\e[HA" — clear line, home, write A
    push(vt, "\e[2K\e[HA");

    // ?screen_row 0 = "A"
    ASSERT_SCREEN_ROW(vt, screen, 0, "A");

    // PUSH "\e[?1049l" — switch back from altscreen
    push(vt, "\e[?1049l");

    // ?screen_row 0 = "P"
    ASSERT_SCREEN_ROW(vt, screen, 0, "P");
}
