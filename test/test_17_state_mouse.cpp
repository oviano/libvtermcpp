// test_17_state_mouse.cpp — state mouse tests
// Ported from upstream libvterm t/17state_mouse.test

#include "harness.h"

// DECRQM on with mouse off
TEST(state_mouse_decrqm_mouse_off)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    output_init(vt);
    state.reset(true);
    callbacks_clear();
    output_clear();

    push(vt, "\e[?1000$p");
    ASSERT_OUTPUT_BYTES("\e[?1000;2$y", 11);

    push(vt, "\e[?1002$p");
    ASSERT_OUTPUT_BYTES("\e[?1002;2$y", 11);

    push(vt, "\e[?1003$p");
    ASSERT_OUTPUT_BYTES("\e[?1003;2$y", 11);
}

// Mouse in simple button report mode
TEST(state_mouse_button_report_mode)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    output_init(vt);
    state.reset(true);

        // RESET fires settermprop for cursorvisible, cursorblink, cursorshape.
    // Now enable mouse click mode.
    callbacks_clear();
    output_clear();

    push(vt, "\e[?1000h");
    // settermprop 8 (MOUSE) = 1 (CLICK)
    ASSERT_EQ(g_cb.settermprop_count, 1);
    ASSERT_EQ(g_cb.settermprop[0].prop, Prop::Mouse);
    ASSERT_EQ(g_cb.settermprop[0].val.number, static_cast<int32_t>(MouseProp::Click));
}

// Press 1
TEST(state_mouse_press_1)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    output_init(vt);
    state.reset(true);
    callbacks_clear();
    output_clear();

    push(vt, "\e[?1000h");
    callbacks_clear();
    output_clear();

    vt.mouse_move(0, 0, Modifier::None);
    vt.mouse_button(1, 1, Modifier::None);
    ASSERT_OUTPUT_BYTES("\e[M\x20\x21\x21", 6);
}

// Release 1
TEST(state_mouse_release_1)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    output_init(vt);
    state.reset(true);
    callbacks_clear();
    output_clear();

    push(vt, "\e[?1000h");
    callbacks_clear();
    output_clear();

    vt.mouse_move(0, 0, Modifier::None);
    vt.mouse_button(1, 1, Modifier::None);
    output_clear();

    vt.mouse_button(1, 0, Modifier::None);
    ASSERT_OUTPUT_BYTES("\e[M\x23\x21\x21", 6);
}

// Ctrl-Press 1
TEST(state_mouse_ctrl_press_1)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    output_init(vt);
    state.reset(true);
    callbacks_clear();
    output_clear();

    push(vt, "\e[?1000h");
    callbacks_clear();
    output_clear();

    vt.mouse_move(0, 0, Modifier::None);

    vt.mouse_button(1, 1, Modifier::Ctrl);
    ASSERT_OUTPUT_BYTES("\e[M\x30\x21\x21", 6);

    vt.mouse_button(1, 0, Modifier::Ctrl);
    ASSERT_OUTPUT_BYTES("\e[M\x33\x21\x21", 6);
}

// Button 2
TEST(state_mouse_button_2)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    output_init(vt);
    state.reset(true);
    callbacks_clear();
    output_clear();

    push(vt, "\e[?1000h");
    callbacks_clear();
    output_clear();

    vt.mouse_move(0, 0, Modifier::None);

    vt.mouse_button(2, 1, Modifier::None);
    ASSERT_OUTPUT_BYTES("\e[M\x21\x21\x21", 6);

    vt.mouse_button(2, 0, Modifier::None);
    ASSERT_OUTPUT_BYTES("\e[M\x23\x21\x21", 6);
}

// Position
TEST(state_mouse_position)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    output_init(vt);
    state.reset(true);
    callbacks_clear();
    output_clear();

    push(vt, "\e[?1000h");
    callbacks_clear();
    output_clear();

    vt.mouse_move(10, 20, Modifier::None);
    vt.mouse_button(1, 1, Modifier::None);
    ASSERT_OUTPUT_BYTES("\e[M\x20\x35\x2b", 6);

    vt.mouse_button(1, 0, Modifier::None);
    ASSERT_OUTPUT_BYTES("\e[M\x23\x35\x2b", 6);

    vt.mouse_move(10, 21, Modifier::None);
    // no output
    ASSERT_EQ(g_output_len, 0);
}

// Wheel events
TEST(state_mouse_wheel_events)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    output_init(vt);
    state.reset(true);
    callbacks_clear();
    output_clear();

    push(vt, "\e[?1000h");
    callbacks_clear();
    output_clear();

    vt.mouse_move(10, 21, Modifier::None);

    vt.mouse_button(4, 1, Modifier::None);
    ASSERT_OUTPUT_BYTES("\e[M\x60\x36\x2b", 6);

    vt.mouse_button(4, 1, Modifier::None);
    ASSERT_OUTPUT_BYTES("\e[M\x60\x36\x2b", 6);

    vt.mouse_button(5, 1, Modifier::None);
    ASSERT_OUTPUT_BYTES("\e[M\x61\x36\x2b", 6);

    vt.mouse_button(6, 1, Modifier::None);
    ASSERT_OUTPUT_BYTES("\e[M\x62\x36\x2b", 6);

    vt.mouse_button(7, 1, Modifier::None);
    ASSERT_OUTPUT_BYTES("\e[M\x63\x36\x2b", 6);
}

// DECRQM on mouse button mode
TEST(state_mouse_decrqm_button_mode)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    output_init(vt);
    state.reset(true);
    callbacks_clear();
    output_clear();

    push(vt, "\e[?1000h");
    callbacks_clear();
    output_clear();

    push(vt, "\e[?1000$p");
    ASSERT_OUTPUT_BYTES("\e[?1000;1$y", 11);

    push(vt, "\e[?1002$p");
    ASSERT_OUTPUT_BYTES("\e[?1002;2$y", 11);

    push(vt, "\e[?1003$p");
    ASSERT_OUTPUT_BYTES("\e[?1003;2$y", 11);
}

// Drag events
TEST(state_mouse_drag_events)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    output_init(vt);
    state.reset(true);

    // RESET already done; enable drag mode
    callbacks_clear();
    output_clear();

    push(vt, "\e[?1002h");
    ASSERT_EQ(g_cb.settermprop_count, 1);
    ASSERT_EQ(g_cb.settermprop[0].prop, Prop::Mouse);
    ASSERT_EQ(g_cb.settermprop[0].val.number, static_cast<int32_t>(MouseProp::Drag));

    callbacks_clear();
    output_clear();

    vt.mouse_move(5, 5, Modifier::None);
    vt.mouse_button(1, 1, Modifier::None);
    ASSERT_OUTPUT_BYTES("\e[M\x20\x26\x26", 6);

    vt.mouse_move(5, 6, Modifier::None);
    ASSERT_OUTPUT_BYTES("\e[M\x40\x27\x26", 6);

    vt.mouse_move(6, 6, Modifier::None);
    ASSERT_OUTPUT_BYTES("\e[M\x40\x27\x27", 6);

    vt.mouse_move(6, 6, Modifier::None);
    // no output - same position
    ASSERT_EQ(g_output_len, 0);

    vt.mouse_button(1, 0, Modifier::None);
    ASSERT_OUTPUT_BYTES("\e[M\x23\x27\x27", 6);

    vt.mouse_move(6, 7, Modifier::None);
    // no output - not dragging
    ASSERT_EQ(g_output_len, 0);
}

// DECRQM on mouse drag mode
TEST(state_mouse_decrqm_drag_mode)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    output_init(vt);
    state.reset(true);
    callbacks_clear();
    output_clear();

    push(vt, "\e[?1002h");
    callbacks_clear();
    output_clear();

    push(vt, "\e[?1000$p");
    ASSERT_OUTPUT_BYTES("\e[?1000;2$y", 11);

    push(vt, "\e[?1002$p");
    ASSERT_OUTPUT_BYTES("\e[?1002;1$y", 11);

    push(vt, "\e[?1003$p");
    ASSERT_OUTPUT_BYTES("\e[?1003;2$y", 11);
}

// Non-drag motion events
TEST(state_mouse_motion_events)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    output_init(vt);
    state.reset(true);
    callbacks_clear();
    output_clear();

    push(vt, "\e[?1002h");
    callbacks_clear();
    output_clear();

    push(vt, "\e[?1003h");
    ASSERT_EQ(g_cb.settermprop_count, 1);
    ASSERT_EQ(g_cb.settermprop[0].prop, Prop::Mouse);
    ASSERT_EQ(g_cb.settermprop[0].val.number, static_cast<int32_t>(MouseProp::Move));

    callbacks_clear();
    output_clear();

    vt.mouse_move(6, 8, Modifier::None);
    ASSERT_OUTPUT_BYTES("\e[M\x43\x29\x27", 6);
}

// DECRQM on mouse motion mode
TEST(state_mouse_decrqm_motion_mode)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    output_init(vt);
    state.reset(true);
    callbacks_clear();
    output_clear();

    push(vt, "\e[?1003h");
    callbacks_clear();
    output_clear();

    push(vt, "\e[?1000$p");
    ASSERT_OUTPUT_BYTES("\e[?1000;2$y", 11);

    push(vt, "\e[?1002$p");
    ASSERT_OUTPUT_BYTES("\e[?1002;2$y", 11);

    push(vt, "\e[?1003$p");
    ASSERT_OUTPUT_BYTES("\e[?1003;1$y", 11);
}

// Bounds checking
TEST(state_mouse_bounds_checking)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    output_init(vt);
    state.reset(true);
    callbacks_clear();
    output_clear();

    push(vt, "\e[?1003h");
    callbacks_clear();
    output_clear();

    vt.mouse_move(300, 300, Modifier::None);
    ASSERT_OUTPUT_BYTES("\e[M\x43\xff\xff", 6);

    vt.mouse_button(1, 1, Modifier::None);
    ASSERT_OUTPUT_BYTES("\e[M\x20\xff\xff", 6);

    vt.mouse_button(1, 0, Modifier::None);
    ASSERT_OUTPUT_BYTES("\e[M\x23\xff\xff", 6);
}

// DECRQM on standard encoding mode
TEST(state_mouse_decrqm_standard_encoding)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    output_init(vt);
    state.reset(true);
    callbacks_clear();
    output_clear();

    push(vt, "\e[?1003h");
    callbacks_clear();
    output_clear();

    push(vt, "\e[?1005$p");
    ASSERT_OUTPUT_BYTES("\e[?1005;2$y", 11);

    push(vt, "\e[?1006$p");
    ASSERT_OUTPUT_BYTES("\e[?1006;2$y", 11);

    push(vt, "\e[?1015$p");
    ASSERT_OUTPUT_BYTES("\e[?1015;2$y", 11);
}

// UTF-8 extended encoding mode
TEST(state_mouse_utf8_encoding)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    output_init(vt);
    state.reset(true);
    callbacks_clear();
    output_clear();

    push(vt, "\e[?1003h");
    callbacks_clear();
    output_clear();

    // Move to 300,300 first (from bounds checking context)
    vt.mouse_move(300, 300, Modifier::None);
    output_clear();

    // 300 + 32 + 1 = 333 = U+014d = \xc5\x8d
    push(vt, "\e[?1005h");
    output_clear();

    vt.mouse_button(1, 1, Modifier::None);
    ASSERT_OUTPUT_BYTES("\e[M\x20\xc5\x8d\xc5\x8d", 8);

    vt.mouse_button(1, 0, Modifier::None);
    ASSERT_OUTPUT_BYTES("\e[M\x23\xc5\x8d\xc5\x8d", 8);
}

// DECRQM on UTF-8 extended encoding mode
TEST(state_mouse_decrqm_utf8_encoding)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    output_init(vt);
    state.reset(true);
    callbacks_clear();
    output_clear();

    push(vt, "\e[?1003h");
    push(vt, "\e[?1005h");
    callbacks_clear();
    output_clear();

    push(vt, "\e[?1005$p");
    ASSERT_OUTPUT_BYTES("\e[?1005;1$y", 11);

    push(vt, "\e[?1006$p");
    ASSERT_OUTPUT_BYTES("\e[?1006;2$y", 11);

    push(vt, "\e[?1015$p");
    ASSERT_OUTPUT_BYTES("\e[?1015;2$y", 11);
}

// SGR extended encoding mode
TEST(state_mouse_sgr_encoding)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    output_init(vt);
    state.reset(true);
    callbacks_clear();
    output_clear();

    push(vt, "\e[?1003h");
    vt.mouse_move(300, 300, Modifier::None);
    output_clear();

    push(vt, "\e[?1006h");
    output_clear();

    vt.mouse_button(1, 1, Modifier::None);
    ASSERT_OUTPUT_BYTES("\e[<0;301;301M", 13);

    vt.mouse_button(1, 0, Modifier::None);
    ASSERT_OUTPUT_BYTES("\e[<0;301;301m", 13);
}

// DECRQM on SGR extended encoding mode
TEST(state_mouse_decrqm_sgr_encoding)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    output_init(vt);
    state.reset(true);
    callbacks_clear();
    output_clear();

    push(vt, "\e[?1003h");
    push(vt, "\e[?1006h");
    callbacks_clear();
    output_clear();

    push(vt, "\e[?1005$p");
    ASSERT_OUTPUT_BYTES("\e[?1005;2$y", 11);

    push(vt, "\e[?1006$p");
    ASSERT_OUTPUT_BYTES("\e[?1006;1$y", 11);

    push(vt, "\e[?1015$p");
    ASSERT_OUTPUT_BYTES("\e[?1015;2$y", 11);
}

// rxvt extended encoding mode
TEST(state_mouse_rxvt_encoding)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    output_init(vt);
    state.reset(true);
    callbacks_clear();
    output_clear();

    push(vt, "\e[?1003h");
    vt.mouse_move(300, 300, Modifier::None);
    output_clear();

    push(vt, "\e[?1015h");
    output_clear();

    vt.mouse_button(1, 1, Modifier::None);
    ASSERT_OUTPUT_BYTES("\e[0;301;301M", 12);

    vt.mouse_button(1, 0, Modifier::None);
    ASSERT_OUTPUT_BYTES("\e[3;301;301M", 12);
}

// DECRQM on rxvt extended encoding mode
TEST(state_mouse_decrqm_rxvt_encoding)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    output_init(vt);
    state.reset(true);
    callbacks_clear();
    output_clear();

    push(vt, "\e[?1003h");
    push(vt, "\e[?1015h");
    callbacks_clear();
    output_clear();

    push(vt, "\e[?1005$p");
    ASSERT_OUTPUT_BYTES("\e[?1005;2$y", 11);

    push(vt, "\e[?1006$p");
    ASSERT_OUTPUT_BYTES("\e[?1006;2$y", 11);

    push(vt, "\e[?1015$p");
    ASSERT_OUTPUT_BYTES("\e[?1015;1$y", 11);
}

// Mouse disabled reports nothing
TEST(state_mouse_disabled_reports_nothing)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    output_init(vt);
    state.reset(true);
    callbacks_clear();
    output_clear();

    // Mouse is off after reset - just move and click
    vt.mouse_move(0, 0, Modifier::None);
    output_clear();

    vt.mouse_button(1, 1, Modifier::None);
    ASSERT_EQ(g_output_len, 0);

    vt.mouse_button(1, 0, Modifier::None);
    ASSERT_EQ(g_output_len, 0);
}

// DECSM can set multiple modes at once
TEST(state_mouse_decsm_multiple_modes)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    output_init(vt);
    state.reset(true);
    callbacks_clear();
    output_clear();

    push(vt, "\e[?1002;1006h");
    // settermprop 8 (MOUSE) = 2 (DRAG)
    ASSERT_EQ(g_cb.settermprop[0].prop, Prop::Mouse);
    ASSERT_EQ(g_cb.settermprop[0].val.number, static_cast<int32_t>(MouseProp::Drag));

    callbacks_clear();
    output_clear();

    vt.mouse_move(0, 0, Modifier::None);
    vt.mouse_button(1, 1, Modifier::None);
    ASSERT_OUTPUT_BYTES("\e[<0;1;1M", 9);
}
