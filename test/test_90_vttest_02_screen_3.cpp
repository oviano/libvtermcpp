// test_90_vttest_02_screen_3.cpp — vttest 02-screen-3: origin mode
// Ported from upstream libvterm .upstream_tests/90vttest_02-screen-3.test

#include "harness.h"

TEST(vttest_02_screen_3_origin_mode)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    Screen& screen = vt.screen();
    screen.set_callbacks(screen_cbs);
    screen.reset(true);

    push(vt, "\e[?6h");
    push(vt, "\e[23;24r");
    push(vt, "\n");
    push(vt, "Bottom");
    push(vt, "\e[1;1H");
    push(vt, "Above");

    ASSERT_SCREEN_ROW(vt, screen, 22, "Above");
    ASSERT_SCREEN_ROW(vt, screen, 23, "Bottom");
}
