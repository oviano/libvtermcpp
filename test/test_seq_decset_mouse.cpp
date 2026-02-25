// test_seq_decset_mouse.cpp -- tests for mouse tracking DECSET modes
//
// Mode 1000: Mouse button click tracking
// Mode 1002: Mouse button+drag tracking
// Mode 1003: Mouse move (any event) tracking
// Mode 1006: SGR extended mouse encoding
// Mode 1015: RXVT extended mouse encoding

#include "harness.h"

// ===== Mode 1000: mouse click tracking =====
TEST(seq_decset_mouse_click)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    push(vt, "\e[?1000h");

    ASSERT_EQ(g_cb.settermprop_count, 1);
    ASSERT_EQ(g_cb.settermprop[0].prop, Prop::Mouse);
    ASSERT_EQ(g_cb.settermprop[0].val.number, static_cast<int32_t>(MouseProp::Click));
}

// ===== Mode 1002: mouse drag tracking =====
TEST(seq_decset_mouse_drag)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    push(vt, "\e[?1002h");

    ASSERT_EQ(g_cb.settermprop_count, 1);
    ASSERT_EQ(g_cb.settermprop[0].prop, Prop::Mouse);
    ASSERT_EQ(g_cb.settermprop[0].val.number, static_cast<int32_t>(MouseProp::Drag));
}

// ===== Mode 1003: mouse move (any event) tracking =====
TEST(seq_decset_mouse_move)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    push(vt, "\e[?1003h");

    ASSERT_EQ(g_cb.settermprop_count, 1);
    ASSERT_EQ(g_cb.settermprop[0].prop, Prop::Mouse);
    ASSERT_EQ(g_cb.settermprop[0].val.number, static_cast<int32_t>(MouseProp::Move));
}

// ===== Disable mode 1000: mouse off =====
TEST(seq_decset_mouse_off)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // First enable, then disable
    push(vt, "\e[?1000h");
    callbacks_clear();

    push(vt, "\e[?1000l");

    ASSERT_EQ(g_cb.settermprop_count, 1);
    ASSERT_EQ(g_cb.settermprop[0].prop, Prop::Mouse);
    ASSERT_EQ(g_cb.settermprop[0].val.number, static_cast<int32_t>(MouseProp::None));
}

// ===== Mode 1006 SGR encoding: mouse output uses SGR format =====
TEST(seq_decset_mouse_sgr_encoding)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    output_init(vt);
    state.reset(true);
    callbacks_clear();

    // Enable mouse click mode + SGR encoding
    push(vt, "\e[?1000h");
    push(vt, "\e[?1006h");

    // Move mouse to row 10, col 20 (0-based) then press button 1
    vt.mouse_move(10, 20, Modifier::None);
    output_clear();
    vt.mouse_button(1, 1, Modifier::None);
    // SGR encoding: \e[<0;21;11M (col+1, row+1)
    ASSERT_OUTPUT_BYTES("\e[<0;21;11M", 11);

    // Release button 1
    output_clear();
    vt.mouse_button(1, 0, Modifier::None);
    // SGR release uses lowercase 'm'
    ASSERT_OUTPUT_BYTES("\e[<0;21;11m", 11);
}

// ===== Mode 1015 RXVT encoding: mouse output uses RXVT format =====
TEST(seq_decset_mouse_rxvt_encoding)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    output_init(vt);
    state.reset(true);
    callbacks_clear();

    // Enable mouse click mode + RXVT encoding
    push(vt, "\e[?1000h");
    push(vt, "\e[?1015h");

    // Move mouse to row 10, col 20 (0-based) then press button 1
    vt.mouse_move(10, 20, Modifier::None);
    output_clear();
    vt.mouse_button(1, 1, Modifier::None);
    // RXVT encoding: \e[0;21;11M (button_code;col+1;row+1 M)
    ASSERT_OUTPUT_BYTES("\e[0;21;11M", 10);

    // Release button 1
    output_clear();
    vt.mouse_button(1, 0, Modifier::None);
    // RXVT release: button code 3
    ASSERT_OUTPUT_BYTES("\e[3;21;11M", 10);
}
