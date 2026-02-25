// test_seq_osc.cpp -- per-sequence tests for OSC (Operating System Command)
//
// OSC sequences set terminal properties like title and icon name:
//   OSC 0 ; text ST  -- set both title and icon name
//   OSC 1 ; text ST  -- set icon name
//   OSC 2 ; text ST  -- set title
//
// OSC can be terminated by either ST (ESC \) or BEL (0x07).

#include "harness.h"

// ============================================================================
// OSC 2 -- set title (ST terminated)
// ============================================================================

// Send OSC 2 with ST terminator, verify Prop::Title is set
TEST(seq_osc_set_title)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    push(vt, "\e]2;Hello World\e\\");

        // String props may arrive as one or more fragments.
    // We verify the first has initial=1 and the last has final=1,
    // and that the property is Prop::Title.
    ASSERT_TRUE(g_cb.settermprop_count >= 1);
    ASSERT_EQ(g_cb.settermprop[0].prop, Prop::Title);
    ASSERT_TRUE(g_cb.settermprop[0].val.string.initial);

    // Verify the final fragment carries the .final flag
    int32_t found_title = 0;
    for (int32_t i = 0; i < g_cb.settermprop_count; i++) {
        if (g_cb.settermprop[i].prop == Prop::Title &&
            g_cb.settermprop[i].val.string.final_) {
            found_title = 1;
        }
    }
    ASSERT_EQ(found_title, 1);
}

// ============================================================================
// OSC 1 -- set icon name
// ============================================================================

// Send OSC 1 with ST terminator, verify Prop::IconName is set
TEST(seq_osc_set_icon_name)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    push(vt, "\e]1;Icon\e\\");

    ASSERT_TRUE(g_cb.settermprop_count >= 1);
    ASSERT_EQ(g_cb.settermprop[0].prop, Prop::IconName);
    ASSERT_TRUE(g_cb.settermprop[0].val.string.initial);

    int32_t found_icon = 0;
    for (int32_t i = 0; i < g_cb.settermprop_count; i++) {
        if (g_cb.settermprop[i].prop == Prop::IconName &&
            g_cb.settermprop[i].val.string.final_) {
            found_icon = 1;
        }
    }
    ASSERT_EQ(found_icon, 1);
}

// ============================================================================
// OSC 0 -- set both title and icon name
// ============================================================================

// OSC 0 sets both Prop::Title and Prop::IconName
TEST(seq_osc_set_title_and_icon)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    push(vt, "\e]0;Both\e\\");

    // Both title and icon name should be set
    int32_t found_title = 0;
    int32_t found_icon = 0;
    for (int32_t i = 0; i < g_cb.settermprop_count; i++) {
        if (g_cb.settermprop[i].prop == Prop::Title &&
            g_cb.settermprop[i].val.string.final_)
            found_title = 1;
        if (g_cb.settermprop[i].prop == Prop::IconName &&
            g_cb.settermprop[i].val.string.final_)
            found_icon = 1;
    }
    ASSERT_EQ(found_title, 1);
    ASSERT_EQ(found_icon, 1);
}

// ============================================================================
// OSC 2 with BEL terminator
// ============================================================================

// BEL (0x07) terminates OSC the same way as ST
TEST(seq_osc_bel_terminator)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    push(vt, "\e]2;Test\x07");

    ASSERT_TRUE(g_cb.settermprop_count >= 1);
    ASSERT_EQ(g_cb.settermprop[0].prop, Prop::Title);
    ASSERT_TRUE(g_cb.settermprop[0].val.string.initial);

    int32_t found_title = 0;
    for (int32_t i = 0; i < g_cb.settermprop_count; i++) {
        if (g_cb.settermprop[i].prop == Prop::Title &&
            g_cb.settermprop[i].val.string.final_) {
            found_title = 1;
        }
    }
    ASSERT_EQ(found_title, 1);
}
