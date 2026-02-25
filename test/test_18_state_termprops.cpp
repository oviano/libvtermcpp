// test_18_state_termprops.cpp — state termprop tests
// Ported from upstream libvterm t/18state_termprops.test

#include "harness.h"

// RESET sets default termprops
TEST(state_termprops_reset)
{
    Terminal vt(25, 80);
    State& state = vt.state();
    state.set_callbacks(state_cbs);

    callbacks_clear();

    state.reset(true);
        // RESET fires:
    //   settermprop 1 true  -> Prop::CursorVisible = true
    //   settermprop 2 true  -> Prop::CursorBlink = true
    //   settermprop 7 1     -> Prop::CursorShape = 1 (BLOCK)
    ASSERT_EQ(g_cb.settermprop_count, 3);
    ASSERT_EQ(g_cb.settermprop[0].prop, Prop::CursorVisible);
    ASSERT_EQ(g_cb.settermprop[0].val.boolean, true);
    ASSERT_EQ(g_cb.settermprop[1].prop, Prop::CursorBlink);
    ASSERT_EQ(g_cb.settermprop[1].val.boolean, true);
    ASSERT_EQ(g_cb.settermprop[2].prop, Prop::CursorShape);
    ASSERT_EQ(g_cb.settermprop[2].val.number, static_cast<int32_t>(CursorShape::Block));
}

// Cursor visibility
TEST(state_termprops_cursor_visible)
{
    Terminal vt(25, 80);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    output_init(vt);

    state.reset(true);

    // PUSH "\e[?25h" -> settermprop 1 true (CURSORVISIBLE)
    callbacks_clear();
    push(vt, "\e[?25h");
    ASSERT_EQ(g_cb.settermprop_count, 1);
    ASSERT_EQ(g_cb.settermprop[0].prop, Prop::CursorVisible);
    ASSERT_EQ(g_cb.settermprop[0].val.boolean, true);

    // PUSH "\e[?25$p" -> output "\e[?25;1$y" (DECRPM: mode set)
    output_clear();
    push(vt, "\e[?25$p");
    ASSERT_OUTPUT_BYTES("\e[?25;1$y", 9);

    // PUSH "\e[?25l" -> settermprop 1 false (CURSORVISIBLE)
    callbacks_clear();
    push(vt, "\e[?25l");
    ASSERT_EQ(g_cb.settermprop_count, 1);
    ASSERT_EQ(g_cb.settermprop[0].prop, Prop::CursorVisible);
    ASSERT_EQ(g_cb.settermprop[0].val.boolean, false);

    // PUSH "\e[?25$p" -> output "\e[?25;2$y" (DECRPM: mode reset)
    output_clear();
    push(vt, "\e[?25$p");
    ASSERT_OUTPUT_BYTES("\e[?25;2$y", 9);
}

// Cursor blink
TEST(state_termprops_cursor_blink)
{
    Terminal vt(25, 80);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    output_init(vt);

    state.reset(true);

    // PUSH "\e[?12h" -> settermprop 2 true (CURSORBLINK)
    callbacks_clear();
    push(vt, "\e[?12h");
    ASSERT_EQ(g_cb.settermprop_count, 1);
    ASSERT_EQ(g_cb.settermprop[0].prop, Prop::CursorBlink);
    ASSERT_EQ(g_cb.settermprop[0].val.boolean, true);

    // PUSH "\e[?12$p" -> output "\e[?12;1$y" (DECRPM: mode set)
    output_clear();
    push(vt, "\e[?12$p");
    ASSERT_OUTPUT_BYTES("\e[?12;1$y", 9);

    // PUSH "\e[?12l" -> settermprop 2 false (CURSORBLINK)
    callbacks_clear();
    push(vt, "\e[?12l");
    ASSERT_EQ(g_cb.settermprop_count, 1);
    ASSERT_EQ(g_cb.settermprop[0].prop, Prop::CursorBlink);
    ASSERT_EQ(g_cb.settermprop[0].val.boolean, false);

    // PUSH "\e[?12$p" -> output "\e[?12;2$y" (DECRPM: mode reset)
    output_clear();
    push(vt, "\e[?12$p");
    ASSERT_OUTPUT_BYTES("\e[?12;2$y", 9);
}

// Cursor shape
TEST(state_termprops_cursor_shape)
{
    Terminal vt(25, 80);
    State& state = vt.state();
    state.set_callbacks(state_cbs);

    state.reset(true);

    // PUSH "\e[3 q" -> settermprop 2 true (CURSORBLINK) + settermprop 7 2 (CURSORSHAPE = UNDERLINE)
    callbacks_clear();
    push(vt, "\e[3 q");
    ASSERT_EQ(g_cb.settermprop_count, 2);
    ASSERT_EQ(g_cb.settermprop[0].prop, Prop::CursorBlink);
    ASSERT_EQ(g_cb.settermprop[0].val.boolean, true);
    ASSERT_EQ(g_cb.settermprop[1].prop, Prop::CursorShape);
    ASSERT_EQ(g_cb.settermprop[1].val.number, static_cast<int32_t>(CursorShape::Underline));
}

// Title
TEST(state_termprops_title)
{
    Terminal vt(25, 80);
    State& state = vt.state();
    state.set_callbacks(state_cbs);

    state.reset(true);

    // PUSH "\e]2;Here is my title\a" -> settermprop 4 ["Here is my title"]
    callbacks_clear();
    push(vt, "\e]2;Here is my title\a");
        // String props may arrive as one or more fragments.
    // The final fragment has .final set. For a single push the
    // harness records each callback invocation separately. With a
    // complete OSC in one push we expect at least one record.
    ASSERT_TRUE(g_cb.settermprop_count >= 1);
    ASSERT_EQ(g_cb.settermprop[0].prop, Prop::Title);
    ASSERT_TRUE(g_cb.settermprop[0].val.string.initial);
    // Check the final fragment carries the .final flag
    ASSERT_TRUE(g_cb.settermprop[g_cb.settermprop_count - 1].val.string.final_);
}

// Title split write
TEST(state_termprops_title_split)
{
    Terminal vt(25, 80);
    State& state = vt.state();
    state.set_callbacks(state_cbs);

    state.reset(true);
    callbacks_clear();

    // First fragment: "\e]2;Here is" — initial, not final
    push(vt, "\e]2;Here is");
    ASSERT_TRUE(g_cb.settermprop_count >= 1);
    ASSERT_EQ(g_cb.settermprop[0].prop, Prop::Title);
    ASSERT_TRUE(g_cb.settermprop[0].val.string.initial);
    // Not yet final since OSC is not terminated
    ASSERT_TRUE(!g_cb.settermprop[g_cb.settermprop_count - 1].val.string.final_);

    // Second fragment: " another title\a" — not initial, final
    int32_t prev_count = g_cb.settermprop_count;
    push(vt, " another title\a");
    ASSERT_TRUE(g_cb.settermprop_count > prev_count);
    ASSERT_EQ(g_cb.settermprop[g_cb.settermprop_count - 1].prop, Prop::Title);
    ASSERT_TRUE(g_cb.settermprop[g_cb.settermprop_count - 1].val.string.final_);
}
