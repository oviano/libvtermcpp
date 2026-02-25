// test_90_vttest_01_movement_4.cpp — vttest 01-movement-4: leading zeroes in ESC sequences
// Ported from upstream libvterm .upstream_tests/90vttest_01-movement-4.test

#include "harness.h"

TEST(vttest_01_movement_4)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    Screen& screen = vt.screen();
    screen.set_callbacks(screen_cbs);
    screen.reset(true);

    push(vt, "\e[00000000004;000000001HT");
    push(vt, "\e[00000000004;000000002Hh");
    push(vt, "\e[00000000004;000000003Hi");
    push(vt, "\e[00000000004;000000004Hs");
    push(vt, "\e[00000000004;000000005H ");
    push(vt, "\e[00000000004;000000006Hi");
    push(vt, "\e[00000000004;000000007Hs");
    push(vt, "\e[00000000004;000000008H ");
    push(vt, "\e[00000000004;000000009Ha");
    push(vt, "\e[00000000004;0000000010H ");
    push(vt, "\e[00000000004;0000000011Hc");
    push(vt, "\e[00000000004;0000000012Ho");
    push(vt, "\e[00000000004;0000000013Hr");
    push(vt, "\e[00000000004;0000000014Hr");
    push(vt, "\e[00000000004;0000000015He");
    push(vt, "\e[00000000004;0000000016Hc");
    push(vt, "\e[00000000004;0000000017Ht");
    push(vt, "\e[00000000004;0000000018H ");
    push(vt, "\e[00000000004;0000000019Hs");
    push(vt, "\e[00000000004;0000000020He");
    push(vt, "\e[00000000004;0000000021Hn");
    push(vt, "\e[00000000004;0000000022Ht");
    push(vt, "\e[00000000004;0000000023He");
    push(vt, "\e[00000000004;0000000024Hn");
    push(vt, "\e[00000000004;0000000025Hc");
    push(vt, "\e[00000000004;0000000026He");

    ASSERT_SCREEN_ROW(vt, screen, 3, "This is a correct sentence");
}
