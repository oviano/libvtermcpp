// test_69_screen_pushline.cpp -- screen scrollback pushline tests
// Ported from upstream libvterm t/69screen_pushline.test

#include "harness.h"

// Spillover text marks continuation on second line
TEST(screen_pushline_continuation_lineinfo)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs_no_scrollrect);
    state.reset(true);
    Screen& screen = vt.screen();
    screen.set_callbacks(screen_cbs_scrollback);

    screen.reset(true);
    callbacks_clear();

    // PUSH "A"x85 — 85 A's fill col 0..79 on row 0, wrap 5 onto row 1
    std::array<char, 86> as{};
    as.fill('A');
    as[85] = '\0';
    push(vt, as.data());

    push(vt, "\r\n");

    // ?lineinfo 0 = (no cont flag)
    ASSERT_LINEINFO(state, 0, continuation, false);

    // ?lineinfo 1 = cont
    ASSERT_LINEINFO(state, 1, continuation, true);
}

// Continuation mark sent to sb_pushline
TEST(screen_pushline_continuation_scrollback)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs_no_scrollrect);
    state.reset(true);
    Screen& screen = vt.screen();
    screen.set_callbacks(screen_cbs_scrollback);

    screen.reset(true);
    callbacks_clear();

    // PUSH "A"x85 — 85 A's, wraps at col 80
    std::array<char, 86> as{};
    as.fill('A');
    as[85] = '\0';
    push(vt, as.data());

    // PUSH "\r\n" — move cursor to row 2 (matching upstream test flow)
    push(vt, "\r\n");

    // PUSH "\n"x23 — scroll enough to push the wrapped pair off the top
    for (int32_t i = 0; i < 23; i++) push(vt, "\n");

    ASSERT_EQ(g_cb.sb_pushline_count, 1);
    ASSERT_EQ(g_cb.sb_pushline[0].cols, 80);
    ASSERT_EQ(g_cb.sb_pushline[0].continuation, false);
    ASSERT_EQ(g_cb.sb_pushline[0].chars[0], 0x41);
    ASSERT_EQ(g_cb.sb_pushline[0].chars[1], 0x41);
    ASSERT_EQ(g_cb.sb_pushline[0].chars[2], 0x41);
    ASSERT_EQ(g_cb.sb_pushline[0].chars[3], 0x41);
    ASSERT_EQ(g_cb.sb_pushline[0].chars[4], 0x41);

    // PUSH "\n" — one more newline pushes the continuation line
    push(vt, "\n");

    ASSERT_EQ(g_cb.sb_pushline_count, 2);
    ASSERT_EQ(g_cb.sb_pushline[1].cols, 80);
    ASSERT_EQ(g_cb.sb_pushline[1].continuation, true);
    ASSERT_EQ(g_cb.sb_pushline[1].chars[0], 0x41);
    ASSERT_EQ(g_cb.sb_pushline[1].chars[1], 0x41);
    ASSERT_EQ(g_cb.sb_pushline[1].chars[2], 0x41);
    ASSERT_EQ(g_cb.sb_pushline[1].chars[3], 0x41);
    ASSERT_EQ(g_cb.sb_pushline[1].chars[4], 0x41);
}
