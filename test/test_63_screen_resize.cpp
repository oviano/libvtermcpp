// test_63_screen_resize.cpp — screen resize tests
// Ported from upstream libvterm t/63screen_resize.test

#include "harness.h"

// Resize wider preserves cells
TEST(screen_resize_wider_preserves_cells)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    Screen& screen = vt.screen();
    screen.set_callbacks(screen_cbs);
    screen.reset(true);
    callbacks_clear();

    push(vt, "AB\r\nCD");

    {
        std::array<uint32_t, 256> chars{};
        Rect rect = { .start_row = 0, .end_row = 1, .start_col = 0, .end_col = 80 };
        size_t len = screen.get_chars(std::span{chars.data(), chars.size()}, rect);
        ASSERT_EQ(len, 2);
        ASSERT_EQ(chars[0], 'A');
        ASSERT_EQ(chars[1], 'B');
    }

    {
        std::array<uint32_t, 256> chars{};
        Rect rect = { .start_row = 1, .end_row = 2, .start_col = 0, .end_col = 80 };
        size_t len = screen.get_chars(std::span{chars.data(), chars.size()}, rect);
        ASSERT_EQ(len, 2);
        ASSERT_EQ(chars[0], 'C');
        ASSERT_EQ(chars[1], 'D');
    }

    vt.set_size(25, 100);

    {
        std::array<uint32_t, 256> chars{};
        Rect rect = { .start_row = 0, .end_row = 1, .start_col = 0, .end_col = 100 };
        size_t len = screen.get_chars(std::span{chars.data(), chars.size()}, rect);
        ASSERT_EQ(len, 2);
        ASSERT_EQ(chars[0], 'A');
        ASSERT_EQ(chars[1], 'B');
    }

    {
        std::array<uint32_t, 256> chars{};
        Rect rect = { .start_row = 1, .end_row = 2, .start_col = 0, .end_col = 100 };
        size_t len = screen.get_chars(std::span{chars.data(), chars.size()}, rect);
        ASSERT_EQ(len, 2);
        ASSERT_EQ(chars[0], 'C');
        ASSERT_EQ(chars[1], 'D');
    }
}

// Resize wider allows print in new area
TEST(screen_resize_wider_allows_print_in_new_area)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    Screen& screen = vt.screen();
    screen.set_callbacks(screen_cbs);
    screen.reset(true);
    callbacks_clear();

    push(vt, "AB\x1b" "[79GCD");

    {
        std::array<uint32_t, 256> chars{};
        Rect rect = { .start_row = 0, .end_row = 1, .start_col = 0, .end_col = 2 };
        size_t len = screen.get_chars(std::span{chars.data(), chars.size()}, rect);
        ASSERT_EQ(len, 2);
        ASSERT_EQ(chars[0], 'A');
        ASSERT_EQ(chars[1], 'B');
    }

    {
        std::array<uint32_t, 256> chars{};
        Rect rect = { .start_row = 0, .end_row = 1, .start_col = 78, .end_col = 80 };
        size_t len = screen.get_chars(std::span{chars.data(), chars.size()}, rect);
        ASSERT_EQ(len, 2);
        ASSERT_EQ(chars[0], 'C');
        ASSERT_EQ(chars[1], 'D');
    }

    vt.set_size(25, 100);

    {
        std::array<uint32_t, 256> chars{};
        Rect rect = { .start_row = 0, .end_row = 1, .start_col = 0, .end_col = 2 };
        size_t len = screen.get_chars(std::span{chars.data(), chars.size()}, rect);
        ASSERT_EQ(len, 2);
        ASSERT_EQ(chars[0], 'A');
        ASSERT_EQ(chars[1], 'B');
    }

    {
        std::array<uint32_t, 256> chars{};
        Rect rect = { .start_row = 0, .end_row = 1, .start_col = 78, .end_col = 80 };
        size_t len = screen.get_chars(std::span{chars.data(), chars.size()}, rect);
        ASSERT_EQ(len, 2);
        ASSERT_EQ(chars[0], 'C');
        ASSERT_EQ(chars[1], 'D');
    }

    push(vt, "E");

    {
        std::array<uint32_t, 256> chars{};
        Rect rect = { .start_row = 0, .end_row = 1, .start_col = 78, .end_col = 81 };
        size_t len = screen.get_chars(std::span{chars.data(), chars.size()}, rect);
        ASSERT_EQ(len, 3);
        ASSERT_EQ(chars[0], 'C');
        ASSERT_EQ(chars[1], 'D');
        ASSERT_EQ(chars[2], 'E');
    }
}

// Resize shorter with blanks just truncates
TEST(screen_resize_shorter_with_blanks_just_truncates)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    Screen& screen = vt.screen();
    screen.set_callbacks(screen_cbs);
    screen.reset(true);
    callbacks_clear();

    push(vt, "Top\x1b" "[10HLine 10");

    ASSERT_SCREEN_ROW(vt, screen, 0, "Top");
    ASSERT_SCREEN_ROW(vt, screen, 9, "Line 10");

    {
        State& state = vt.state();
        ASSERT_CURSOR(state, 9, 7);
    }

    vt.set_size(20, 80);

    ASSERT_SCREEN_ROW(vt, screen, 0, "Top");
    ASSERT_SCREEN_ROW(vt, screen, 9, "Line 10");

    {
        State& state = vt.state();
        ASSERT_CURSOR(state, 9, 7);
    }
}

// Resize shorter with content must scroll
TEST(screen_resize_shorter_with_content_must_scroll)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    Screen& screen = vt.screen();
    screen.set_callbacks(screen_cbs_scrollback);

    screen.reset(true);
    callbacks_clear();

    push(vt, "Top\x1b" "[25HLine 25\x1b" "[15H");

    ASSERT_SCREEN_ROW(vt, screen, 0, "Top");
    ASSERT_SCREEN_ROW(vt, screen, 24, "Line 25");

    {
        State& state = vt.state();
        ASSERT_CURSOR(state, 14, 0);
    }

    callbacks_clear();

    vt.set_size(20, 80);

    ASSERT_EQ(g_cb.sb_pushline_count, 5);
    ASSERT_EQ(g_cb.sb_pushline[0].cols, 80);
    ASSERT_EQ(g_cb.sb_pushline[0].chars[0], 0x54); // T
    ASSERT_EQ(g_cb.sb_pushline[0].chars[1], 0x6F); // o
    ASSERT_EQ(g_cb.sb_pushline[0].chars[2], 0x70); // p

    ASSERT_EQ(g_cb.sb_pushline[1].cols, 80);
    ASSERT_EQ(g_cb.sb_pushline[1].chars[0], 0);

    ASSERT_EQ(g_cb.sb_pushline[2].cols, 80);
    ASSERT_EQ(g_cb.sb_pushline[2].chars[0], 0);

    ASSERT_EQ(g_cb.sb_pushline[3].cols, 80);
    ASSERT_EQ(g_cb.sb_pushline[3].chars[0], 0);

    ASSERT_EQ(g_cb.sb_pushline[4].cols, 80);
    ASSERT_EQ(g_cb.sb_pushline[4].chars[0], 0);

    ASSERT_SCREEN_ROW(vt, screen, 0, "");
    ASSERT_SCREEN_ROW(vt, screen, 19, "Line 25");

    {
        State& state = vt.state();
        ASSERT_CURSOR(state, 9, 0);
    }
}

// Resize shorter does not lose line with cursor
TEST(screen_resize_shorter_does_not_lose_line_with_cursor)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    Screen& screen = vt.screen();
    screen.set_callbacks(screen_cbs_scrollback);

    screen.reset(true);
    callbacks_clear();

    push(vt, "\x1b" "[24HLine 24\r\nLine 25\r\n");

    ASSERT_EQ(g_cb.sb_pushline_count, 1);
    ASSERT_EQ(g_cb.sb_pushline[0].cols, 80);
    ASSERT_EQ(g_cb.sb_pushline[0].chars[0], 0);

    ASSERT_SCREEN_ROW(vt, screen, 23, "Line 25");

    {
        State& state = vt.state();
        ASSERT_CURSOR(state, 24, 0);
    }

    callbacks_clear();

    vt.set_size(24, 80);

    ASSERT_EQ(g_cb.sb_pushline_count, 1);
    ASSERT_EQ(g_cb.sb_pushline[0].cols, 80);
    ASSERT_EQ(g_cb.sb_pushline[0].chars[0], 0);

    ASSERT_SCREEN_ROW(vt, screen, 22, "Line 25");

    {
        State& state = vt.state();
        ASSERT_CURSOR(state, 23, 0);
    }
}

// Resize shorter does not send the cursor to a negative row
TEST(screen_resize_shorter_does_not_send_cursor_to_negative_row)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    Screen& screen = vt.screen();
    screen.set_callbacks(screen_cbs_scrollback);

    screen.reset(true);
    callbacks_clear();

    push(vt, "\x1b" "[24HLine 24\r\nLine 25\x1b" "[H");

    {
        State& state = vt.state();
        ASSERT_CURSOR(state, 0, 0);
    }

    callbacks_clear();

    vt.set_size(20, 80);

    ASSERT_EQ(g_cb.sb_pushline_count, 5);
    ASSERT_EQ(g_cb.sb_pushline[0].cols, 80);
    ASSERT_EQ(g_cb.sb_pushline[0].chars[0], 0);
    ASSERT_EQ(g_cb.sb_pushline[1].cols, 80);
    ASSERT_EQ(g_cb.sb_pushline[1].chars[0], 0);
    ASSERT_EQ(g_cb.sb_pushline[2].cols, 80);
    ASSERT_EQ(g_cb.sb_pushline[2].chars[0], 0);
    ASSERT_EQ(g_cb.sb_pushline[3].cols, 80);
    ASSERT_EQ(g_cb.sb_pushline[3].chars[0], 0);
    ASSERT_EQ(g_cb.sb_pushline[4].cols, 80);
    ASSERT_EQ(g_cb.sb_pushline[4].chars[0], 0);

    {
        State& state = vt.state();
        ASSERT_CURSOR(state, 0, 0);
    }
}

// Resize taller attempts to pop scrollback
TEST(screen_resize_taller_attempts_to_pop_scrollback)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    Screen& screen = vt.screen();
    screen.set_callbacks(screen_cbs_scrollback);

    screen.reset(true);
    callbacks_clear();

    push(vt, "Line 1\x1b" "[25HBottom\x1b" "[15H");

    ASSERT_SCREEN_ROW(vt, screen, 0, "Line 1");
    ASSERT_SCREEN_ROW(vt, screen, 24, "Bottom");

    {
        State& state = vt.state();
        ASSERT_CURSOR(state, 14, 0);
    }

    callbacks_clear();

    vt.set_size(30, 80);

    ASSERT_EQ(g_cb.sb_popline_count, 5);

    ASSERT_SCREEN_ROW(vt, screen, 0, "ABCDE");
    ASSERT_SCREEN_ROW(vt, screen, 5, "Line 1");
    ASSERT_SCREEN_ROW(vt, screen, 29, "Bottom");

    {
        State& state = vt.state();
        ASSERT_CURSOR(state, 19, 0);
    }
}

// Resize can operate on altscreen
TEST(screen_resize_can_operate_on_altscreen)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    Screen& screen = vt.screen();
    screen.enable_altscreen(true);
    screen.set_callbacks(screen_cbs);
    screen.reset(true);
    callbacks_clear();

    push(vt, "Main screen\x1b" "[?1049h\x1b" "[HAlt screen");

    vt.set_size(30, 80);

    ASSERT_SCREEN_ROW(vt, screen, 0, "Alt screen");

    push(vt, "\x1b" "[?1049l");

    ASSERT_SCREEN_ROW(vt, screen, 0, "Main screen");
}
