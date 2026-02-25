// test_32_state_flow.cpp — state flow/continuation tests
// Ported from upstream libvterm t/32state_flow.test

#include "harness.h"

// Many of these test cases inspired by
//   https://blueprints.launchpad.net/libvterm/+spec/reflow-cases

// Spillover text marks continuation on second line
TEST(state_flow_spillover_marks_continuation)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);

    state.reset(true);
    callbacks_clear();

    // PUSH "A"x100
    push(vt,
        "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA" // 50
        "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"); // 50
    push(vt, "\r\n");
    ASSERT_LINEINFO(state, 0, continuation, false);
    ASSERT_LINEINFO(state, 1, continuation, true);
}

// CRLF in column 80 does not mark continuation
TEST(state_flow_crlf_col80_no_continuation)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);

    state.reset(true);
    callbacks_clear();

    // PUSH "B"x80
    push(vt,
        "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB");
    push(vt, "\r\n");
    // PUSH "B"x20
    push(vt, "BBBBBBBBBBBBBBBBBBBB");
    push(vt, "\r\n");
    ASSERT_LINEINFO(state, 0, continuation, false);
    ASSERT_LINEINFO(state, 1, continuation, false);
}

// EL cancels continuation of following line
TEST(state_flow_el_cancels_continuation)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);

    state.reset(true);
    callbacks_clear();

    // PUSH "D"x100
    push(vt,
        "DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD" // 98
        "DD"); // +2=100
    ASSERT_LINEINFO(state, 1, continuation, true);

    // Reverse index, move to col 79, erase to end of line
    push(vt, "\x1b" "M" "\x1b" "[79G" "\x1b" "[K");
    ASSERT_LINEINFO(state, 1, continuation, false);
}
