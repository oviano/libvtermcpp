// test_92_regression.cpp — regression tests
// Ported from upstream libvterm t/92lp1640917.test

#include "harness.h"

// Regression: idempotent DECSM 1002 (button-event mouse tracking) must not
// break mouse-move reporting that was already active.
TEST(regression_lp1640917_idempotent_decsm_1002)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs_no_scrollrect);
    state.reset(true);
    output_init(vt);
    callbacks_clear();

    // Enable button-event mouse tracking (DECSM 1002)
    push(vt, "\e[?1002h");
    output_clear();

    // Move to (0,0) and press button 1
    vt.mouse_move(0, 0, Modifier::None);
    vt.mouse_button(1, true, Modifier::None);
    {
        constexpr std::string_view expected = "\e[M\x20\x21\x21";
        ASSERT_OUTPUT_BYTES(expected.data(), expected.size());
    }

    // Drag to (1,0) — button still held
    vt.mouse_move(1, 0, Modifier::None);
    {
        constexpr std::string_view expected = "\e[M\x40\x21\x22";
        ASSERT_OUTPUT_BYTES(expected.data(), expected.size());
    }

    // Idempotent re-enable of DECSM 1002 — must not break drag reporting
    push(vt, "\e[?1002h");
    output_clear();

    // Drag to (2,0) — must still produce a move event
    vt.mouse_move(2, 0, Modifier::None);
    {
        constexpr std::string_view expected = "\e[M\x40\x21\x23";
        ASSERT_OUTPUT_BYTES(expected.data(), expected.size());
    }
}
