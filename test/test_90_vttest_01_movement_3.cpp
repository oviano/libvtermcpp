// test_90_vttest_01_movement_3.cpp — cursor-control characters inside ESC sequences
// Ported from upstream libvterm t/90vttest_01-movement-3.test

#include "harness.h"

TEST(vttest_01_movement_3_ctrl_in_csi)
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

    // Row 0: plain "A B C D E F G H I"
    push(vt, "A B C D E F G H I");
    push(vt, "\x0d\x0a");

    // Row 1: BS embedded in CSI
    push(vt, "A\e[2\x08" "CB\e[2\x08" "CC\e[2\x08" "CD\e[2\x08" "CE\e[2\x08" "CF\e[2\x08" "CG\e[2\x08" "CH\e[2\x08" "CI");
    push(vt, "\x0d\x0a");

    // Row 2: CR embedded in CSI
    push(vt, "A \e[\x0d" "2CB\e[\x0d" "4CC\e[\x0d" "6CD\e[\x0d" "8CE\e[\x0d" "10CF\e[\x0d" "12CG\e[\x0d" "14CH\e[\x0d" "16CI");
    push(vt, "\x0d\x0a");

    // Row 3: VT embedded in CSI
    push(vt, "A \e[1\x0b" "AB \e[1\x0b" "AC \e[1\x0b" "AD \e[1\x0b" "AE \e[1\x0b" "AF \e[1\x0b" "AG \e[1\x0b" "AH \e[1\x0b" "AI \e[1\x0b" "A");

    for (int32_t row = 0; row <= 2; row++) {
        ASSERT_SCREEN_ROW(vt, screen, row, "A B C D E F G H I");
    }

    ASSERT_SCREEN_ROW(vt, screen, 3, "A B C D E F G H I");

    ASSERT_CURSOR(state, 3, 18);
}
