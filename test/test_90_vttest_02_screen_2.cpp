// test_90_vttest_02_screen_2.cpp — vttest 02-screen-2: TAB setting/resetting
// Ported from upstream libvterm .upstream_tests/90vttest_02-screen-2.test

#include "harness.h"

// TAB setting/resetting
TEST(vttest_02_screen_2_tab_setting_resetting)
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

    push(vt, "\x1b[2J\x1b[3g");

    push(vt, "\x1b[1;1H");
    for (int32_t i = 0; i < 26; i++)
        push(vt, "\x1b[3C\x1bH");

    push(vt, "\x1b[1;4H");
    for (int32_t i = 0; i < 13; i++)
        push(vt, "\x1b[0g\x1b[6C");

    push(vt, "\x1b[1;7H");
    push(vt, "\x1b[1g\x1b[2g");

    push(vt, "\x1b[1;1H");
    for (int32_t i = 0; i < 13; i++)
        push(vt, "\t*");

    push(vt, "\x1b[2;2H");
    for (int32_t i = 0; i < 13; i++)
        push(vt, "     *");

    ASSERT_SCREEN_ROW(vt, screen, 0,
        "      *     *     *     *     *     *     *     *     *     *     *     *     *");
    ASSERT_SCREEN_ROW(vt, screen, 1,
        "      *     *     *     *     *     *     *     *     *     *     *     *     *");

    ASSERT_CURSOR(state, 1, 79);
}
