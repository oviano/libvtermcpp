// test_22_state_save.cpp — state save/restore tests
// Ported from upstream libvterm t/22state_save.test

#include "harness.h"

// Set up state
TEST(state_save_set_up_state)
{
    Terminal vt(25, 80);
    State& state = vt.state();
    state.set_callbacks(state_cbs);

    // RESET
    callbacks_clear();
    state.reset(true);
    ASSERT_EQ(g_cb.settermprop_count, 3);
    ASSERT_EQ(g_cb.settermprop[0].prop, Prop::CursorVisible);
    ASSERT_EQ(g_cb.settermprop[0].val.boolean, true);
    ASSERT_EQ(g_cb.settermprop[1].prop, Prop::CursorBlink);
    ASSERT_EQ(g_cb.settermprop[1].val.boolean, true);
    ASSERT_EQ(g_cb.settermprop[2].prop, Prop::CursorShape);
    ASSERT_EQ(g_cb.settermprop[2].val.number, static_cast<int32_t>(CursorShape::Block));

    callbacks_clear();

    push(vt, "\e[2;2H");
    ASSERT_CURSOR(state, 1, 1);

    push(vt, "\e[1m");
    ASSERT_PEN_BOOL(state, Attr::Bold, true);
}

// Save
TEST(state_save_save)
{
    // no assertions — verifying no crash
    Terminal vt(25, 80);
    State& state = vt.state();
    state.set_callbacks(state_cbs);

    state.reset(true);
    callbacks_clear();

    // Set up state
    push(vt, "\e[2;2H");
    push(vt, "\e[1m");

    // Save
    push(vt, "\e[?1048h");
}

// Change state
TEST(state_save_change_state)
{
    Terminal vt(25, 80);
    State& state = vt.state();
    state.set_callbacks(state_cbs);

    state.reset(true);
    callbacks_clear();

    // Set up state
    push(vt, "\e[2;2H");
    push(vt, "\e[1m");

    // Save
    push(vt, "\e[?1048h");

    // Change state
    push(vt, "\e[5;5H");
    ASSERT_CURSOR(state, 4, 4);

    callbacks_clear();
    push(vt, "\e[4 q");
    ASSERT_EQ(g_cb.settermprop_count, 2);
    ASSERT_EQ(g_cb.settermprop[0].prop, Prop::CursorBlink);
    ASSERT_EQ(g_cb.settermprop[0].val.boolean, false);
    ASSERT_EQ(g_cb.settermprop[1].prop, Prop::CursorShape);
    ASSERT_EQ(g_cb.settermprop[1].val.number, static_cast<int32_t>(CursorShape::Underline));

    push(vt, "\e[22;4m");
    ASSERT_PEN_BOOL(state, Attr::Bold, false);
    ASSERT_PEN_INT(state, Attr::Underline, 1);
}

// Restore
TEST(state_save_restore)
{
    Terminal vt(25, 80);
    State& state = vt.state();
    state.set_callbacks(state_cbs);

    state.reset(true);
    callbacks_clear();

    // Set up state
    push(vt, "\e[2;2H");
    push(vt, "\e[1m");

    // Save
    push(vt, "\e[?1048h");

    // Change state
    push(vt, "\e[5;5H");
    push(vt, "\e[4 q");
    push(vt, "\e[22;4m");

    // Restore
    callbacks_clear();
    push(vt, "\e[?1048l");
    ASSERT_EQ(g_cb.settermprop_count, 3);
    ASSERT_EQ(g_cb.settermprop[0].prop, Prop::CursorVisible);
    ASSERT_EQ(g_cb.settermprop[0].val.boolean, true);
    ASSERT_EQ(g_cb.settermprop[1].prop, Prop::CursorBlink);
    ASSERT_EQ(g_cb.settermprop[1].val.boolean, true);
    ASSERT_EQ(g_cb.settermprop[2].prop, Prop::CursorShape);
    ASSERT_EQ(g_cb.settermprop[2].val.number, static_cast<int32_t>(CursorShape::Block));
    ASSERT_CURSOR(state, 1, 1);
    ASSERT_PEN_BOOL(state, Attr::Bold, true);
    ASSERT_PEN_INT(state, Attr::Underline, 0);
}

// Save/restore using DECSC/DECRC
TEST(state_save_decsc_decrc)
{
    Terminal vt(25, 80);
    State& state = vt.state();
    state.set_callbacks(state_cbs);

    state.reset(true);
    callbacks_clear();

    // Set up state, save, change, restore (to get back to known state)
    push(vt, "\e[2;2H");
    push(vt, "\e[1m");
    push(vt, "\e[?1048h");
    push(vt, "\e[5;5H");
    push(vt, "\e[4 q");
    push(vt, "\e[22;4m");
    push(vt, "\e[?1048l");

    // Save/restore using DECSC/DECRC
    push(vt, "\e[2;2H\e7");
    ASSERT_CURSOR(state, 1, 1);

    push(vt, "\e[5;5H");
    ASSERT_CURSOR(state, 4, 4);

    callbacks_clear();
    push(vt, "\e8");
    ASSERT_EQ(g_cb.settermprop_count, 3);
    ASSERT_EQ(g_cb.settermprop[0].prop, Prop::CursorVisible);
    ASSERT_EQ(g_cb.settermprop[0].val.boolean, true);
    ASSERT_EQ(g_cb.settermprop[1].prop, Prop::CursorBlink);
    ASSERT_EQ(g_cb.settermprop[1].val.boolean, true);
    ASSERT_EQ(g_cb.settermprop[2].prop, Prop::CursorShape);
    ASSERT_EQ(g_cb.settermprop[2].val.number, static_cast<int32_t>(CursorShape::Block));
    ASSERT_CURSOR(state, 1, 1);
}

// Save twice, restore twice happens on both edge transitions
TEST(state_save_twice_restore_twice)
{
    Terminal vt(25, 80);
    State& state = vt.state();
    state.set_callbacks(state_cbs);

    state.reset(true);
    callbacks_clear();

    // Set up state, save, change, restore (to get back to known state)
    push(vt, "\e[2;2H");
    push(vt, "\e[1m");
    push(vt, "\e[?1048h");
    push(vt, "\e[5;5H");
    push(vt, "\e[4 q");
    push(vt, "\e[22;4m");
    push(vt, "\e[?1048l");

    // Continue through DECSC/DECRC section
    push(vt, "\e[2;2H\e7");
    push(vt, "\e[5;5H");
    push(vt, "\e8");

    // Save twice, restore twice
    push(vt, "\e[2;10H\e[?1048h\e[6;10H\e[?1048h");

    push(vt, "\e[H");
    ASSERT_CURSOR(state, 0, 0);

    callbacks_clear();
    push(vt, "\e[?1048l");
    ASSERT_EQ(g_cb.settermprop_count, 3);
    ASSERT_EQ(g_cb.settermprop[0].prop, Prop::CursorVisible);
    ASSERT_EQ(g_cb.settermprop[0].val.boolean, true);
    ASSERT_EQ(g_cb.settermprop[1].prop, Prop::CursorBlink);
    ASSERT_EQ(g_cb.settermprop[1].val.boolean, true);
    ASSERT_EQ(g_cb.settermprop[2].prop, Prop::CursorShape);
    ASSERT_EQ(g_cb.settermprop[2].val.number, static_cast<int32_t>(CursorShape::Block));
    ASSERT_CURSOR(state, 5, 9);

    push(vt, "\e[H");
    ASSERT_CURSOR(state, 0, 0);

    callbacks_clear();
    push(vt, "\e[?1048l");
    ASSERT_EQ(g_cb.settermprop_count, 3);
    ASSERT_EQ(g_cb.settermprop[0].prop, Prop::CursorVisible);
    ASSERT_EQ(g_cb.settermprop[0].val.boolean, true);
    ASSERT_EQ(g_cb.settermprop[1].prop, Prop::CursorBlink);
    ASSERT_EQ(g_cb.settermprop[1].val.boolean, true);
    ASSERT_EQ(g_cb.settermprop[2].prop, Prop::CursorShape);
    ASSERT_EQ(g_cb.settermprop[2].val.number, static_cast<int32_t>(CursorShape::Block));
    ASSERT_CURSOR(state, 5, 9);
}
