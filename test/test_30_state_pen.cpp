// test_30_state_pen.cpp — state pen attribute tests
// Ported from upstream libvterm t/30state_pen.test

#include "harness.h"

// Reset
TEST(state_pen_reset)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.set_bold_highbright(true);
    state.reset(true);

    push(vt, "\e[m");
    ASSERT_PEN_BOOL(state, Attr::Bold, false);
    ASSERT_PEN_INT(state, Attr::Underline, 0);
    ASSERT_PEN_BOOL(state, Attr::Italic, false);
    ASSERT_PEN_BOOL(state, Attr::Blink, false);
    ASSERT_PEN_BOOL(state, Attr::Reverse, false);
    ASSERT_PEN_INT(state, Attr::Font, 0);
    ASSERT_PEN_COLOR_DEFAULT_FG(state, Attr::Foreground, 240, 240, 240);
    ASSERT_PEN_COLOR_DEFAULT_BG(state, Attr::Background, 0, 0, 0);
}

// Bold
TEST(state_pen_bold)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.set_bold_highbright(true);
    state.reset(true);

    push(vt, "\e[1m");
    ASSERT_PEN_BOOL(state, Attr::Bold, true);

    push(vt, "\e[22m");
    ASSERT_PEN_BOOL(state, Attr::Bold, false);

    push(vt, "\e[1m\e[m");
    ASSERT_PEN_BOOL(state, Attr::Bold, false);
}

// Underline
TEST(state_pen_underline)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.set_bold_highbright(true);
    state.reset(true);

    push(vt, "\e[4m");
    ASSERT_PEN_INT(state, Attr::Underline, 1);

    push(vt, "\e[21m");
    ASSERT_PEN_INT(state, Attr::Underline, 2);

    push(vt, "\e[24m");
    ASSERT_PEN_INT(state, Attr::Underline, 0);

    push(vt, "\e[4m\e[4:0m");
    ASSERT_PEN_INT(state, Attr::Underline, 0);

    push(vt, "\e[4:1m");
    ASSERT_PEN_INT(state, Attr::Underline, 1);

    push(vt, "\e[4:2m");
    ASSERT_PEN_INT(state, Attr::Underline, 2);

    push(vt, "\e[4:3m");
    ASSERT_PEN_INT(state, Attr::Underline, 3);

    push(vt, "\e[4m\e[m");
    ASSERT_PEN_INT(state, Attr::Underline, 0);
}

// Italic
TEST(state_pen_italic)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.set_bold_highbright(true);
    state.reset(true);

    push(vt, "\e[3m");
    ASSERT_PEN_BOOL(state, Attr::Italic, true);

    push(vt, "\e[23m");
    ASSERT_PEN_BOOL(state, Attr::Italic, false);

    push(vt, "\e[3m\e[m");
    ASSERT_PEN_BOOL(state, Attr::Italic, false);
}

// Blink
TEST(state_pen_blink)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.set_bold_highbright(true);
    state.reset(true);

    push(vt, "\e[5m");
    ASSERT_PEN_BOOL(state, Attr::Blink, true);

    push(vt, "\e[25m");
    ASSERT_PEN_BOOL(state, Attr::Blink, false);

    push(vt, "\e[5m\e[m");
    ASSERT_PEN_BOOL(state, Attr::Blink, false);
}

// Reverse
TEST(state_pen_reverse)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.set_bold_highbright(true);
    state.reset(true);

    push(vt, "\e[7m");
    ASSERT_PEN_BOOL(state, Attr::Reverse, true);

    push(vt, "\e[27m");
    ASSERT_PEN_BOOL(state, Attr::Reverse, false);

    push(vt, "\e[7m\e[m");
    ASSERT_PEN_BOOL(state, Attr::Reverse, false);
}

// Font Selection
TEST(state_pen_font_selection)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.set_bold_highbright(true);
    state.reset(true);

    push(vt, "\e[11m");
    ASSERT_PEN_INT(state, Attr::Font, 1);

    push(vt, "\e[19m");
    ASSERT_PEN_INT(state, Attr::Font, 9);

    push(vt, "\e[10m");
    ASSERT_PEN_INT(state, Attr::Font, 0);

    push(vt, "\e[11m\e[m");
    ASSERT_PEN_INT(state, Attr::Font, 0);
}

// Foreground
TEST(state_pen_foreground)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.set_bold_highbright(true);
    state.reset(true);

    push(vt, "\e[31m");
    ASSERT_PEN_COLOR_IDX(state, Attr::Foreground, 1);

    push(vt, "\e[32m");
    ASSERT_PEN_COLOR_IDX(state, Attr::Foreground, 2);

    push(vt, "\e[34m");
    ASSERT_PEN_COLOR_IDX(state, Attr::Foreground, 4);

    push(vt, "\e[91m");
    ASSERT_PEN_COLOR_IDX(state, Attr::Foreground, 9);

    push(vt, "\e[38:2:10:20:30m");
    ASSERT_PEN_COLOR_RGB(state, Attr::Foreground, 10, 20, 30);

    push(vt, "\e[38:5:1m");
    ASSERT_PEN_COLOR_IDX(state, Attr::Foreground, 1);

    push(vt, "\e[39m");
    ASSERT_PEN_COLOR_DEFAULT_FG(state, Attr::Foreground, 240, 240, 240);
}

// Background
TEST(state_pen_background)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.set_bold_highbright(true);
    state.reset(true);

    push(vt, "\e[41m");
    ASSERT_PEN_COLOR_IDX(state, Attr::Background, 1);

    push(vt, "\e[42m");
    ASSERT_PEN_COLOR_IDX(state, Attr::Background, 2);

    push(vt, "\e[44m");
    ASSERT_PEN_COLOR_IDX(state, Attr::Background, 4);

    push(vt, "\e[101m");
    ASSERT_PEN_COLOR_IDX(state, Attr::Background, 9);

    push(vt, "\e[48:2:10:20:30m");
    ASSERT_PEN_COLOR_RGB(state, Attr::Background, 10, 20, 30);

    push(vt, "\e[48:5:1m");
    ASSERT_PEN_COLOR_IDX(state, Attr::Background, 1);

    push(vt, "\e[49m");
    ASSERT_PEN_COLOR_DEFAULT_BG(state, Attr::Background, 0, 0, 0);
}

// Bold+ANSI colour == highbright
TEST(state_pen_bold_highbright)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.set_bold_highbright(true);
    state.reset(true);

    push(vt, "\e[m\e[1;37m");
    ASSERT_PEN_BOOL(state, Attr::Bold, true);
    ASSERT_PEN_COLOR_IDX(state, Attr::Foreground, 15);

    push(vt, "\e[m\e[37;1m");
    ASSERT_PEN_BOOL(state, Attr::Bold, true);
    ASSERT_PEN_COLOR_IDX(state, Attr::Foreground, 15);
}

// Super/Subscript
TEST(state_pen_super_subscript)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.set_bold_highbright(true);
    state.reset(true);

    push(vt, "\e[73m");
    ASSERT_PEN_BOOL(state, Attr::Small, true);
    ASSERT_PEN_INT(state, Attr::Baseline, to_underlying(Baseline::Raise));

    push(vt, "\e[74m");
    ASSERT_PEN_BOOL(state, Attr::Small, true);
    ASSERT_PEN_INT(state, Attr::Baseline, to_underlying(Baseline::Lower));

    push(vt, "\e[75m");
    ASSERT_PEN_BOOL(state, Attr::Small, false);
    ASSERT_PEN_INT(state, Attr::Baseline, to_underlying(Baseline::Normal));
}

// DECSTR resets pen attributes
TEST(state_pen_decstr_reset)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.set_bold_highbright(true);
    state.reset(true);

    push(vt, "\e[1;4m");
    ASSERT_PEN_BOOL(state, Attr::Bold, true);
    ASSERT_PEN_INT(state, Attr::Underline, 1);

    push(vt, "\e[!p");
    ASSERT_PEN_BOOL(state, Attr::Bold, false);
    ASSERT_PEN_INT(state, Attr::Underline, 0);
}
