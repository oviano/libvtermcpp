// test_seq_decscusr.cpp -- per-sequence tests for DECSCUSR (CSI Ps SP q)
//
// DECSCUSR (Set Cursor Style) selects the cursor shape and blink mode:
//   CSI 0 SP q  or  CSI 1 SP q  -- blinking block
//   CSI 2 SP q                   -- steady block
//   CSI 3 SP q                   -- blinking underline
//   CSI 4 SP q                   -- steady underline
//   CSI 5 SP q                   -- blinking bar (left)
//   CSI 6 SP q                   -- steady bar (left)
//
// Each DECSCUSR fires two settermprop callbacks:
//   1. Prop::CursorBlink (boolean)
//   2. Prop::CursorShape (int: BLOCK=1, UNDERLINE=2, BAR_LEFT=3)

#include "harness.h"

// ============================================================================
// DECSCUSR 1 -- blinking block
// ============================================================================

TEST(seq_decscusr_block_blink)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    push(vt, "\e[1 q");

    ASSERT_EQ(g_cb.settermprop_count, 2);
    ASSERT_EQ(g_cb.settermprop[0].prop, Prop::CursorBlink);
    ASSERT_EQ(g_cb.settermprop[0].val.boolean, true);
    ASSERT_EQ(g_cb.settermprop[1].prop, Prop::CursorShape);
    ASSERT_EQ(g_cb.settermprop[1].val.number, static_cast<int32_t>(CursorShape::Block));
}

// ============================================================================
// DECSCUSR 2 -- steady block
// ============================================================================

TEST(seq_decscusr_block_steady)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    push(vt, "\e[2 q");

    ASSERT_EQ(g_cb.settermprop_count, 2);
    ASSERT_EQ(g_cb.settermprop[0].prop, Prop::CursorBlink);
    ASSERT_EQ(g_cb.settermprop[0].val.boolean, false);
    ASSERT_EQ(g_cb.settermprop[1].prop, Prop::CursorShape);
    ASSERT_EQ(g_cb.settermprop[1].val.number, static_cast<int32_t>(CursorShape::Block));
}

// ============================================================================
// DECSCUSR 3 -- blinking underline
// ============================================================================

TEST(seq_decscusr_underline_blink)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    push(vt, "\e[3 q");

    ASSERT_EQ(g_cb.settermprop_count, 2);
    ASSERT_EQ(g_cb.settermprop[0].prop, Prop::CursorBlink);
    ASSERT_EQ(g_cb.settermprop[0].val.boolean, true);
    ASSERT_EQ(g_cb.settermprop[1].prop, Prop::CursorShape);
    ASSERT_EQ(g_cb.settermprop[1].val.number, static_cast<int32_t>(CursorShape::Underline));
}

// ============================================================================
// DECSCUSR 4 -- steady underline
// ============================================================================

TEST(seq_decscusr_underline_steady)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    push(vt, "\e[4 q");

    ASSERT_EQ(g_cb.settermprop_count, 2);
    ASSERT_EQ(g_cb.settermprop[0].prop, Prop::CursorBlink);
    ASSERT_EQ(g_cb.settermprop[0].val.boolean, false);
    ASSERT_EQ(g_cb.settermprop[1].prop, Prop::CursorShape);
    ASSERT_EQ(g_cb.settermprop[1].val.number, static_cast<int32_t>(CursorShape::Underline));
}

// ============================================================================
// DECSCUSR 5 -- blinking bar (left)
// ============================================================================

TEST(seq_decscusr_bar_blink)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    push(vt, "\e[5 q");

    ASSERT_EQ(g_cb.settermprop_count, 2);
    ASSERT_EQ(g_cb.settermprop[0].prop, Prop::CursorBlink);
    ASSERT_EQ(g_cb.settermprop[0].val.boolean, true);
    ASSERT_EQ(g_cb.settermprop[1].prop, Prop::CursorShape);
    ASSERT_EQ(g_cb.settermprop[1].val.number, static_cast<int32_t>(CursorShape::BarLeft));
}

// ============================================================================
// DECSCUSR 6 -- steady bar (left)
// ============================================================================

TEST(seq_decscusr_bar_steady)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    push(vt, "\e[6 q");

    ASSERT_EQ(g_cb.settermprop_count, 2);
    ASSERT_EQ(g_cb.settermprop[0].prop, Prop::CursorBlink);
    ASSERT_EQ(g_cb.settermprop[0].val.boolean, false);
    ASSERT_EQ(g_cb.settermprop[1].prop, Prop::CursorShape);
    ASSERT_EQ(g_cb.settermprop[1].val.number, static_cast<int32_t>(CursorShape::BarLeft));
}

// ============================================================================
// DECSCUSR 0 -- default (same as 1: blinking block)
// ============================================================================

// CSI 0 SP q is treated as CSI 1 SP q in libvterm: blinking block.
TEST(seq_decscusr_default)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);

    // First set a different style so the default is actually a change
    push(vt, "\e[4 q");
    callbacks_clear();

    // Now send CSI 0 SP q -- should reset to blinking block
    push(vt, "\e[0 q");

    ASSERT_EQ(g_cb.settermprop_count, 2);
    ASSERT_EQ(g_cb.settermprop[0].prop, Prop::CursorBlink);
    ASSERT_EQ(g_cb.settermprop[0].val.boolean, true);
    ASSERT_EQ(g_cb.settermprop[1].prop, Prop::CursorShape);
    ASSERT_EQ(g_cb.settermprop[1].val.number, static_cast<int32_t>(CursorShape::Block));
}
