// test_seq_da.cpp -- per-sequence tests for DA (CSI c) and DA2 (CSI > c)
//
// DA  (Device Attributes, primary):   CSI c  or  CSI 0 c
//   Response: CSI ? 1 ; 2 c  (DEC VT100 with AVO)
//
// DA2 (Device Attributes, secondary): CSI > c
//   Response: CSI > 0 ; 100 ; 0 c

#include "harness.h"

// ============================================================================
// DA primary -- CSI c
// ============================================================================

// CSI c should produce the primary DA response: ESC [ ? 1 ; 2 c
TEST(seq_da_primary)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    output_init(vt);
    state.reset(true);
    callbacks_clear();
    output_clear();

    push(vt, "\e[c");
    ASSERT_OUTPUT_BYTES("\e[?1;2c", 7);
}

// ============================================================================
// DA primary with explicit 0 param
// ============================================================================

// CSI 0 c should produce the same response as CSI c
TEST(seq_da_primary_default_param)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    output_init(vt);
    state.reset(true);
    callbacks_clear();
    output_clear();

    push(vt, "\e[0c");
    ASSERT_OUTPUT_BYTES("\e[?1;2c", 7);
}

// ============================================================================
// DA primary explicit 0 again
// ============================================================================

// Verify CSI 0 c is identical to CSI c
TEST(seq_da_primary_explicit_0)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    output_init(vt);
    state.reset(true);
    callbacks_clear();
    output_clear();

    // Send CSI 0 c
    push(vt, "\e[0c");
    ASSERT_OUTPUT_BYTES("\e[?1;2c", 7);

    // Immediately send CSI c and verify same response
    output_clear();
    push(vt, "\e[c");
    ASSERT_OUTPUT_BYTES("\e[?1;2c", 7);
}

// ============================================================================
// DA2 secondary -- CSI > c
// ============================================================================

// CSI > c should produce the secondary DA response: ESC [ > 0 ; 100 ; 0 c
TEST(seq_da_secondary)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    output_init(vt);
    state.reset(true);
    callbacks_clear();
    output_clear();

    push(vt, "\e[>c");
    ASSERT_OUTPUT_BYTES("\e[>0;100;0c", 11);
}
