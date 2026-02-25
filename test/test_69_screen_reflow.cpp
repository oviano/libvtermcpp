// test_69_screen_reflow.cpp — screen reflow tests
// Ported from upstream libvterm t/69screen_reflow.test

#include "harness.h"

// Resize wider reflows wide lines
TEST(screen_reflow_resize_wider)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs_no_scrollrect);
    state.reset(true);
    Screen& screen = vt.screen();
    screen.set_callbacks(screen_cbs);
    screen.enable_reflow(true);
    vt.set_size(5, 10);
    screen.reset(true);
    callbacks_clear();

    push(vt, "AAAAAAAAAAAA");

    ASSERT_SCREEN_ROW(vt, screen, 0, "AAAAAAAAAA");
    ASSERT_SCREEN_ROW(vt, screen, 1, "AA");
    ASSERT_LINEINFO(state, 1, continuation, true);
    ASSERT_CURSOR(state, 1, 2);

    vt.set_size(5, 15);

    ASSERT_SCREEN_ROW(vt, screen, 0, "AAAAAAAAAAAA");
    ASSERT_SCREEN_ROW(vt, screen, 1, "");
    ASSERT_LINEINFO(state, 1, continuation, false);
    ASSERT_CURSOR(state, 0, 12);

    vt.set_size(5, 20);

    ASSERT_SCREEN_ROW(vt, screen, 0, "AAAAAAAAAAAA");
    ASSERT_SCREEN_ROW(vt, screen, 1, "");
    ASSERT_LINEINFO(state, 1, continuation, false);
    ASSERT_CURSOR(state, 0, 12);
}

// Resize narrower can create continuation lines
TEST(screen_reflow_resize_narrower)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs_no_scrollrect);
    state.reset(true);
    Screen& screen = vt.screen();
    screen.set_callbacks(screen_cbs);
    screen.enable_reflow(true);
    vt.set_size(5, 10);
    screen.reset(true);
    callbacks_clear();

    push(vt, "ABCDEFGHI");

    ASSERT_SCREEN_ROW(vt, screen, 0, "ABCDEFGHI");
    ASSERT_SCREEN_ROW(vt, screen, 1, "");
    ASSERT_LINEINFO(state, 1, continuation, false);
    ASSERT_CURSOR(state, 0, 9);

    vt.set_size(5, 8);

    ASSERT_SCREEN_ROW(vt, screen, 0, "ABCDEFGH");
    ASSERT_SCREEN_ROW(vt, screen, 1, "I");
    ASSERT_LINEINFO(state, 1, continuation, true);
    ASSERT_CURSOR(state, 1, 1);

    vt.set_size(5, 6);

    ASSERT_SCREEN_ROW(vt, screen, 0, "ABCDEF");
    ASSERT_SCREEN_ROW(vt, screen, 1, "GHI");
    ASSERT_LINEINFO(state, 1, continuation, true);
    ASSERT_CURSOR(state, 1, 3);
}

// Shell wrapped prompt behaviour
TEST(screen_reflow_shell_wrapped_prompt)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs_no_scrollrect);
    state.reset(true);
    Screen& screen = vt.screen();
    screen.set_callbacks(screen_cbs);
    screen.enable_reflow(true);
    vt.set_size(5, 10);
    screen.reset(true);
    callbacks_clear();

    push(vt, "PROMPT GOES HERE\r\n> \r\n\r\nPROMPT GOES HERE\r\n> ");

    ASSERT_SCREEN_ROW(vt, screen, 0, ">");
    ASSERT_SCREEN_ROW(vt, screen, 1, "");
    ASSERT_SCREEN_ROW(vt, screen, 2, "PROMPT GOE");
    ASSERT_SCREEN_ROW(vt, screen, 3, "S HERE");
    ASSERT_LINEINFO(state, 3, continuation, true);
    ASSERT_SCREEN_ROW(vt, screen, 4, ">");
    ASSERT_CURSOR(state, 4, 2);

    vt.set_size(5, 11);

    ASSERT_SCREEN_ROW(vt, screen, 0, ">");
    ASSERT_SCREEN_ROW(vt, screen, 1, "");
    ASSERT_SCREEN_ROW(vt, screen, 2, "PROMPT GOES");
    ASSERT_SCREEN_ROW(vt, screen, 3, " HERE");
    ASSERT_LINEINFO(state, 3, continuation, true);
    ASSERT_SCREEN_ROW(vt, screen, 4, ">");
    ASSERT_CURSOR(state, 4, 2);

    vt.set_size(5, 12);

    ASSERT_SCREEN_ROW(vt, screen, 0, ">");
    ASSERT_SCREEN_ROW(vt, screen, 1, "");
    ASSERT_SCREEN_ROW(vt, screen, 2, "PROMPT GOES");
    ASSERT_SCREEN_ROW(vt, screen, 3, "HERE");
    ASSERT_LINEINFO(state, 3, continuation, true);
    ASSERT_SCREEN_ROW(vt, screen, 4, ">");
    ASSERT_CURSOR(state, 4, 2);

    vt.set_size(5, 16);

    ASSERT_SCREEN_ROW(vt, screen, 0, ">");
    ASSERT_SCREEN_ROW(vt, screen, 1, "");
    ASSERT_SCREEN_ROW(vt, screen, 2, "PROMPT GOES HERE");
    ASSERT_LINEINFO(state, 3, continuation, false);
    ASSERT_SCREEN_ROW(vt, screen, 3, ">");
    ASSERT_CURSOR(state, 3, 2);
}

// Cursor goes missing
// For more context: https://github.com/neovim/neovim/pull/21124
TEST(screen_reflow_cursor_goes_missing)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs_no_scrollrect);
    state.reset(true);
    Screen& screen = vt.screen();
    screen.set_callbacks(screen_cbs);
    screen.enable_reflow(true);
    vt.set_size(5, 10);
    screen.reset(true);
    callbacks_clear();

    vt.set_size(5, 5);
    vt.set_size(3, 1);
    push(vt, "\x1b" "[2;1Habc\r\n\x1b" "[H");
    vt.set_size(1, 1);

    ASSERT_CURSOR(state, 0, 0);
}
