// test_90_vttest_01_movement_2.cpp — vttest 01 movement test 2 (scroll region)
// Ported from upstream libvterm t/90vttest_01-movement-2.test

#include "harness.h"

// vttest 01 movement-2: scroll region fill and scroll
TEST(vttest_01_movement_2)
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

    push(vt, "\e[3;21r");
    push(vt, "\e[?6h");

    push(vt, "\e[19;1HA\e[19;80Ha\x0a\e[18;80HaB\e[19;80HB\b b\x0a\e[19;80HC\b\b\t\tc\e[19;2H\bC\x0a\e[19;80H\x0a\e[18;1HD\e[18;80Hd");
    push(vt, "\e[19;1HE\e[19;80He\x0a\e[18;80HeF\e[19;80HF\b f\x0a\e[19;80HG\b\b\t\tg\e[19;2H\bG\x0a\e[19;80H\x0a\e[18;1HH\e[18;80Hh");
    push(vt, "\e[19;1HI\e[19;80Hi\x0a\e[18;80HiJ\e[19;80HJ\b j\x0a\e[19;80HK\b\b\t\tk\e[19;2H\bK\x0a\e[19;80H\x0a\e[18;1HL\e[18;80Hl");
    push(vt, "\e[19;1HM\e[19;80Hm\x0a\e[18;80HmN\e[19;80HN\b n\x0a\e[19;80HO\b\b\t\to\e[19;2H\bO\x0a\e[19;80H\x0a\e[18;1HP\e[18;80Hp");
    push(vt, "\e[19;1HQ\e[19;80Hq\x0a\e[18;80HqR\e[19;80HR\b r\x0a\e[19;80HS\b\b\t\ts\e[19;2H\bS\x0a\e[19;80H\x0a\e[18;1HT\e[18;80Ht");
    push(vt, "\e[19;1HU\e[19;80Hu\x0a\e[18;80HuV\e[19;80HV\b v\x0a\e[19;80HW\b\b\t\tw\e[19;2H\bW\x0a\e[19;80H\x0a\e[18;1HX\e[18;80Hx");
    push(vt, "\e[19;1HY\e[19;80Hy\x0a\e[18;80HyZ\e[19;80HZ\b z\x0a");

    ASSERT_SCREEN_ROW(vt, screen,  2, "I                                                                              i");
    ASSERT_SCREEN_ROW(vt, screen,  3, "J                                                                              j");
    ASSERT_SCREEN_ROW(vt, screen,  4, "K                                                                              k");
    ASSERT_SCREEN_ROW(vt, screen,  5, "L                                                                              l");
    ASSERT_SCREEN_ROW(vt, screen,  6, "M                                                                              m");
    ASSERT_SCREEN_ROW(vt, screen,  7, "N                                                                              n");
    ASSERT_SCREEN_ROW(vt, screen,  8, "O                                                                              o");
    ASSERT_SCREEN_ROW(vt, screen,  9, "P                                                                              p");
    ASSERT_SCREEN_ROW(vt, screen, 10, "Q                                                                              q");
    ASSERT_SCREEN_ROW(vt, screen, 11, "R                                                                              r");
    ASSERT_SCREEN_ROW(vt, screen, 12, "S                                                                              s");
    ASSERT_SCREEN_ROW(vt, screen, 13, "T                                                                              t");
    ASSERT_SCREEN_ROW(vt, screen, 14, "U                                                                              u");
    ASSERT_SCREEN_ROW(vt, screen, 15, "V                                                                              v");
    ASSERT_SCREEN_ROW(vt, screen, 16, "W                                                                              w");
    ASSERT_SCREEN_ROW(vt, screen, 17, "X                                                                              x");
    ASSERT_SCREEN_ROW(vt, screen, 18, "Y                                                                              y");
    ASSERT_SCREEN_ROW(vt, screen, 19, "Z                                                                              z");
    ASSERT_SCREEN_ROW(vt, screen, 20, "");

    ASSERT_CURSOR(state, 20, 79);
}
