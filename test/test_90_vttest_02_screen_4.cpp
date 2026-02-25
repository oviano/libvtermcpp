// test_90_vttest_02_screen_4.cpp — vttest 02 screen test 4 (origin mode disabled)
// Ported from upstream libvterm t/90vttest_02-screen-4.test

#include "harness.h"

// vttest 02 screen-4: origin mode (2) — with origin mode disabled
TEST(vttest_02_screen_4)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    Screen& screen = vt.screen();
    screen.set_callbacks(screen_cbs);
    screen.reset(true);

    push(vt, "\e[?6l");
    push(vt, "\e[23;24r");
    push(vt, "\e[24;1H");
    push(vt, "Bottom");
    push(vt, "\e[1;1H");
    push(vt, "Top");

    ASSERT_SCREEN_ROW(vt, screen, 23, "Bottom");
    ASSERT_SCREEN_ROW(vt, screen,  0, "Top");
}
