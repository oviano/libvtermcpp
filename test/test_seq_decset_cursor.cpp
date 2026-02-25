// test_seq_decset_cursor.cpp -- tests for cursor-related DECSET modes
//
// Mode 1: DECCKM (Application Cursor Keys)
// Mode 5: DECSCNM (Reverse Video / Screen Mode)
// Mode 25: DECTCEM (Text Cursor Enable Mode)

#include "harness.h"

// ===== DECCKM application cursor mode: arrow keys send SS3 sequences =====
TEST(seq_decset_decckm_app_cursor)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    output_init(vt);
    state.reset(true);
    callbacks_clear();

    // Enable application cursor mode: CSI ? 1 h
    push(vt, "\e[?1h");

    // Up arrow should produce SS3 A (\eOA)
    output_clear();
    vt.keyboard_key(Key::Up, Modifier::None);
    ASSERT_OUTPUT_BYTES("\eOA", 3);

    // Down arrow should produce SS3 B (\eOB)
    output_clear();
    vt.keyboard_key(Key::Down, Modifier::None);
    ASSERT_OUTPUT_BYTES("\eOB", 3);

    // Right arrow should produce SS3 C (\eOC)
    output_clear();
    vt.keyboard_key(Key::Right, Modifier::None);
    ASSERT_OUTPUT_BYTES("\eOC", 3);

    // Left arrow should produce SS3 D (\eOD)
    output_clear();
    vt.keyboard_key(Key::Left, Modifier::None);
    ASSERT_OUTPUT_BYTES("\eOD", 3);
}

// ===== DECCKM normal cursor mode: arrow keys send CSI sequences =====
TEST(seq_decset_decckm_normal_cursor)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    output_init(vt);
    state.reset(true);
    callbacks_clear();

    // Ensure normal cursor mode (default): CSI ? 1 l
    push(vt, "\e[?1l");

    // Up arrow should produce CSI A (\e[A)
    output_clear();
    vt.keyboard_key(Key::Up, Modifier::None);
    ASSERT_OUTPUT_BYTES("\e[A", 3);

    // Down arrow should produce CSI B (\e[B)
    output_clear();
    vt.keyboard_key(Key::Down, Modifier::None);
    ASSERT_OUTPUT_BYTES("\e[B", 3);

    // Right arrow should produce CSI C (\e[C)
    output_clear();
    vt.keyboard_key(Key::Right, Modifier::None);
    ASSERT_OUTPUT_BYTES("\e[C", 3);

    // Left arrow should produce CSI D (\e[D)
    output_clear();
    vt.keyboard_key(Key::Left, Modifier::None);
    ASSERT_OUTPUT_BYTES("\e[D", 3);
}

// ===== DECSCNM reverse video: settermprop REVERSE=1 =====
TEST(seq_decset_decscnm_reverse)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Enable reverse video: CSI ? 5 h
    push(vt, "\e[?5h");

    ASSERT_EQ(g_cb.settermprop_count, 1);
    ASSERT_EQ(g_cb.settermprop[0].prop, Prop::Reverse);
    ASSERT_EQ(g_cb.settermprop[0].val.boolean, true);
}

// ===== DECSCNM normal video: settermprop REVERSE=0 =====
TEST(seq_decset_decscnm_normal)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // First enable, then disable reverse video
    push(vt, "\e[?5h");
    callbacks_clear();

    push(vt, "\e[?5l");

    ASSERT_EQ(g_cb.settermprop_count, 1);
    ASSERT_EQ(g_cb.settermprop[0].prop, Prop::Reverse);
    ASSERT_EQ(g_cb.settermprop[0].val.boolean, false);
}

// ===== DECTCEM cursor visible: settermprop CURSORVISIBLE=1 =====
TEST(seq_decset_dectcem_visible)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // First hide, then show cursor to get a clean callback
    push(vt, "\e[?25l");
    callbacks_clear();

    push(vt, "\e[?25h");

    ASSERT_EQ(g_cb.settermprop_count, 1);
    ASSERT_EQ(g_cb.settermprop[0].prop, Prop::CursorVisible);
    ASSERT_EQ(g_cb.settermprop[0].val.boolean, true);
}

// ===== DECTCEM cursor hidden: settermprop CURSORVISIBLE=0 =====
TEST(seq_decset_dectcem_hidden)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Hide cursor: CSI ? 25 l
    push(vt, "\e[?25l");

    ASSERT_EQ(g_cb.settermprop_count, 1);
    ASSERT_EQ(g_cb.settermprop[0].prop, Prop::CursorVisible);
    ASSERT_EQ(g_cb.settermprop[0].val.boolean, false);
}
