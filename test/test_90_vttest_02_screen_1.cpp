// test_90_vttest_02_screen_1.cpp — vttest screen-1 wrap-around mode test
// Ported from upstream libvterm .upstream_tests/90vttest_02-screen-1.test

#include "harness.h"

TEST(vttest_02_screen_1_wrap_around_mode)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    Screen& screen = vt.screen();
    screen.set_callbacks(screen_cbs);
    screen.reset(true);

    // Enable wrap-around mode (DECAWM) and push 170 asterisks.
    push(vt, "\e[?7h");
    {
        std::array<char, 171> stars{};
        stars.fill('*');
        stars[170] = '\0';
        push(vt, stars.data());
    }

    // Disable wrap-around mode, move to row 3 col 1, push 177 asterisks.
    push(vt, "\e[?7l\e[3;1H");
    {
        std::array<char, 178> stars{};
        stars.fill('*');
        stars[177] = '\0';
        push(vt, stars.data());
    }

    // Re-enable wrap-around mode, move to row 5 col 1, write "OK".
    push(vt, "\e[?7h\e[5;1HOK");

    ASSERT_SCREEN_ROW(vt, screen, 0, "********************************************************************************");
    ASSERT_SCREEN_ROW(vt, screen, 1, "********************************************************************************");
    ASSERT_SCREEN_ROW(vt, screen, 2, "********************************************************************************");
    ASSERT_SCREEN_ROW(vt, screen, 3, "");
    ASSERT_SCREEN_ROW(vt, screen, 4, "OK");
}
