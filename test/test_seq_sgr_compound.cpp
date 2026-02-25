// test_seq_sgr_compound.cpp -- tests for SGR edge cases and compound sequences
//
// Covers:
//   - Multiple SGR parameters in a single sequence
//   - Empty SGR (no params) as attribute reset
//   - Colon-separated underline style sub-parameters
//   - Mid-sequence reset (SGR 0) interaction with other params

#include "harness.h"

// Multiple SGR params in one sequence: \e[1;3;4m sets bold+italic+underline
TEST(seq_sgr_multiple_params)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    push(vt, "\e[1;3;4m");
    ASSERT_PEN_BOOL(state, Attr::Bold, true);
    ASSERT_PEN_BOOL(state, Attr::Italic, true);
    ASSERT_PEN_INT(state, Attr::Underline, 1);
}

// Empty SGR (no params) resets all attributes
TEST(seq_sgr_empty_is_reset)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Set bold first
    push(vt, "\e[1m");
    ASSERT_PEN_BOOL(state, Attr::Bold, true);

    // \e[m with no params should reset
    push(vt, "\e[m");
    ASSERT_PEN_BOOL(state, Attr::Bold, false);
}

// Colon-separated underline style: \e[4:3m sets curly underline
TEST(seq_sgr_colon_underline_style)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // 4:3 = curly underline (UnderlineCurly = 3)
    push(vt, "\e[4:3m");
    ASSERT_PEN_INT(state, Attr::Underline, 3);

    // 4:0 = underline off
    push(vt, "\e[4:0m");
    ASSERT_PEN_INT(state, Attr::Underline, 0);
}

// Mid-sequence reset: \e[1;0;3m -- bold on, reset all, italic on
TEST(seq_sgr_mixed_reset_in_sequence)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    push(vt, "\e[1;0;3m");
    // The 0 in the middle resets bold, then 3 sets italic
    ASSERT_PEN_BOOL(state, Attr::Bold, false);
    ASSERT_PEN_BOOL(state, Attr::Italic, true);
}
