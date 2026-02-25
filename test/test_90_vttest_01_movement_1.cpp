// test_90_vttest_01_movement_1.cpp — vttest 01-movement-1 screen drawing test
// Ported from upstream libvterm .upstream_tests/90vttest_01-movement-1.test

#include "harness.h"

TEST(vttest_01_movement_1)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs_no_scrollrect);
    state.reset(true);
    Screen& screen = vt.screen();
    screen.set_callbacks(screen_cbs);
    screen.reset(true);
    callbacks_clear();

    // PUSH "\e#8" — DECALN: fill screen with 'E'
    push(vt, "\x1b#8");

    // PUSH "\e[9;10H\e[1J"
    push(vt, "\x1b[9;10H\x1b[1J");

    // PUSH "\e[18;60H\e[0J\e[1K"
    push(vt, "\x1b[18;60H\x1b[0J\x1b[1K");

    // PUSH "\e[9;71H\e[0K"
    push(vt, "\x1b[9;71H\x1b[0K");

    // $SEQ 10 16
    for (int32_t i = 10; i <= 16; i++) {
        auto buf = std::format("\x1b[{};10H\x1b[1K\x1b[{};71H\x1b[0K", i, i);
        push(vt, buf);
    }

    // PUSH "\e[17;30H\e[2K"
    push(vt, "\x1b[17;30H\x1b[2K");

    // $SEQ 1 80
    for (int32_t i = 1; i <= 80; i++) {
        auto buf = std::format("\x1b[24;{}f*\x1b[1;{}f*", i, i);
        push(vt, buf);
    }

    // PUSH "\e[2;2H"
    push(vt, "\x1b[2;2H");

    // $REP 22
    for (int32_t i = 0; i < 22; i++)
        push(vt, "+\x1b[1D\x1b" "D");

    // PUSH "\e[23;79H"
    push(vt, "\x1b[23;79H");

    // $REP 22
    for (int32_t i = 0; i < 22; i++)
        push(vt, "+\x1b[1D\x1bM");

    // PUSH "\e[2;1H"
    push(vt, "\x1b[2;1H");

    // $SEQ 2 23
    for (int32_t i = 2; i <= 23; i++) {
        auto buf = std::format("*\x1b[{};80H*\x1b[10D\x1b" "E", i);
        push(vt, buf);
    }

    // PUSH "\e[2;10H\e[42D\e[2C"
    push(vt, "\x1b[2;10H\x1b[42D\x1b[2C");

    // $REP 76
    for (int32_t i = 0; i < 76; i++)
        push(vt, "+\x1b[0C\x1b[2D\x1b[1C");

    // PUSH "\e[23;70H\e[42C\e[2D"
    push(vt, "\x1b[23;70H\x1b[42C\x1b[2D");

    // $REP 76
    for (int32_t i = 0; i < 76; i++)
        push(vt, "+\x1b[1D\x1b[1C\x1b[0D\x08");

    push(vt, "\x1b[1;1H");
    push(vt, "\x1b[10A");
    push(vt, "\x1b[1A");
    push(vt, "\x1b[0A");
    push(vt, "\x1b[24;80H");
    push(vt, "\x1b[10B");
    push(vt, "\x1b[1B");
    push(vt, "\x1b[0B");
    push(vt, "\x1b[10;12H");

    for (int32_t i = 0; i < 58; i++) push(vt, " ");
    push(vt, "\x1b[1B\x1b[58D");

    for (int32_t i = 0; i < 58; i++) push(vt, " ");
    push(vt, "\x1b[1B\x1b[58D");

    for (int32_t i = 0; i < 58; i++) push(vt, " ");
    push(vt, "\x1b[1B\x1b[58D");

    for (int32_t i = 0; i < 58; i++) push(vt, " ");
    push(vt, "\x1b[1B\x1b[58D");

    for (int32_t i = 0; i < 58; i++) push(vt, " ");
    push(vt, "\x1b[1B\x1b[58D");

    for (int32_t i = 0; i < 58; i++) push(vt, " ");
    push(vt, "\x1b[1B\x1b[58D");

    push(vt, "\x1b[5A\x1b[1CThe screen should be cleared,  and have an unbroken bor-");
    push(vt, "\x1b[12;13Hder of *'s and +'s around the edge,   and exactly in the");
    push(vt, "\x1b[13;13Hmiddle  there should be a frame of E's around this  text");
    push(vt, "\x1b[14;13Hwith  one (1) free position around it.    Push <RETURN>");

    ASSERT_SCREEN_ROW(vt, screen, 0,
        "********************************************************************************");

    ASSERT_SCREEN_ROW(vt, screen, 1,
        "*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*");

    for (int32_t i = 2; i <= 7; i++)
        ASSERT_SCREEN_ROW(vt, screen, i,
            "*+                                                                            +*");

    ASSERT_SCREEN_ROW(vt, screen, 8,
        "*+        EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE        +*");

    ASSERT_SCREEN_ROW(vt, screen, 9,
        "*+        E                                                          E        +*");

    ASSERT_SCREEN_ROW(vt, screen, 10,
        "*+        E The screen should be cleared,  and have an unbroken bor- E        +*");

    ASSERT_SCREEN_ROW(vt, screen, 11,
        "*+        E der of *'s and +'s around the edge,   and exactly in the E        +*");

    ASSERT_SCREEN_ROW(vt, screen, 12,
        "*+        E middle  there should be a frame of E's around this  text E        +*");

    ASSERT_SCREEN_ROW(vt, screen, 13,
        "*+        E with  one (1) free position around it.    Push <RETURN>  E        +*");

    ASSERT_SCREEN_ROW(vt, screen, 14,
        "*+        E                                                          E        +*");

    ASSERT_SCREEN_ROW(vt, screen, 15,
        "*+        EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE        +*");

    for (int32_t i = 16; i <= 21; i++)
        ASSERT_SCREEN_ROW(vt, screen, i,
            "*+                                                                            +*");

    ASSERT_SCREEN_ROW(vt, screen, 22,
        "*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*");

    ASSERT_SCREEN_ROW(vt, screen, 23,
        "********************************************************************************");

    ASSERT_CURSOR(state, 13, 67);
}
