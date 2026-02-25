// test_seq_deckpam_deckpnm.cpp -- tests for DECKPAM (ESC =) and DECKPNM (ESC >)
//
// DECKPAM (Application Keypad Mode) causes keypad keys to generate
// application-mode escape sequences (e.g., KP_0 -> ESC O p).
// DECKPNM (Numeric Keypad Mode) causes keypad keys to generate
// their numeric characters (e.g., KP_0 -> '0').
//
// The default after reset is numeric mode.

#include "harness.h"

// ===== DECKPAM: application mode keypad output =====
TEST(seq_deckpam_app_mode)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    output_init(vt);
    state.reset(true);
    callbacks_clear();

    // Enable application keypad mode: ESC =
    push(vt, "\e=");

    // Send KP_0 key
    output_clear();
    vt.keyboard_key(Key::KP0, Modifier::None);

    // In application mode, KP_0 should produce ESC O p
    ASSERT_OUTPUT_BYTES("\eOp", 3);
}

// ===== DECKPNM: numeric mode keypad output =====
TEST(seq_deckpnm_numeric_mode)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    output_init(vt);
    state.reset(true);
    callbacks_clear();

    // Explicitly set numeric keypad mode: ESC >
    push(vt, "\e>");

    // Send KP_0 key
    output_clear();
    vt.keyboard_key(Key::KP0, Modifier::None);

    // In numeric mode, KP_0 should produce '0'
    ASSERT_OUTPUT_BYTES("0", 1);
}

// ===== DECKPAM/DECKPNM toggle: app then numeric =====
TEST(seq_deckpam_toggle)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    output_init(vt);
    state.reset(true);
    callbacks_clear();

    // Default is numeric mode -- verify
    output_clear();
    vt.keyboard_key(Key::KP0, Modifier::None);
    ASSERT_OUTPUT_BYTES("0", 1);

    // Switch to application mode
    push(vt, "\e=");

    output_clear();
    vt.keyboard_key(Key::KP0, Modifier::None);
    ASSERT_OUTPUT_BYTES("\eOp", 3);

    // Switch back to numeric mode
    push(vt, "\e>");

    output_clear();
    vt.keyboard_key(Key::KP0, Modifier::None);
    ASSERT_OUTPUT_BYTES("0", 1);
}
