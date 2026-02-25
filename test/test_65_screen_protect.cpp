// test_65_screen_protect.cpp -- screen layer protection tests
// Ported from upstream libvterm t/65screen_protect.test

#include "harness.h"

// Selective erase
TEST(screen_protect_selective_erase)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    Screen& screen = vt.screen();
    screen.set_callbacks(screen_cbs);
    screen.reset(true);

    // Write "A", set protection, write "B", unset protection, write "C"
    push(vt, "A\e[1\"qB\e[\"qC");
    ASSERT_SCREEN_ROW(vt, screen, 0, "ABC");

    // Move to column 1, then selective erase to end of display
    push(vt, "\e[G\e[?J");
    ASSERT_SCREEN_ROW(vt, screen, 0, " B");
}

// Non-selective erase
TEST(screen_protect_non_selective_erase)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    Screen& screen = vt.screen();
    screen.set_callbacks(screen_cbs);
    screen.reset(true);

    // Write "A", set protection, write "B", unset protection, write "C"
    push(vt, "A\e[1\"qB\e[\"qC");
    ASSERT_SCREEN_ROW(vt, screen, 0, "ABC");

    // Move to column 1, then non-selective erase to end of display
    push(vt, "\e[G\e[J");
    ASSERT_SCREEN_ROW(vt, screen, 0, "");
}
