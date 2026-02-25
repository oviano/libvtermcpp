// test_seq_decstr_ris.cpp -- tests for DECSTR (CSI ! p) soft reset and RIS (ESC c) hard reset
//
// RIS (Reset to Initial State) is a hard reset that:
//   - Homes the cursor to (0,0)
//   - Erases the entire screen
//   - Cancels scroll regions
//   - Resets all modes to defaults
//
// DECSTR (Soft Terminal Reset) resets SGR attributes, DECOM, DECAWM,
// and other modes, but does NOT erase the screen or home the cursor.

#include "harness.h"

// ===== RIS homes cursor to (0,0) =====
TEST(seq_ris_homes_cursor)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Move cursor to row 10, col 20 (1-based: 11;21)
    push(vt, "\e[11;21H");
    ASSERT_CURSOR(state, 10, 20);

    // RIS: ESC c
    push(vt, "\ec");
    ASSERT_CURSOR(state, 0, 0);
}

// ===== RIS erases screen =====
TEST(seq_ris_erases_screen)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Print some text
    push(vt, "Hello, world!");

    // RIS
    callbacks_clear();
    push(vt, "\ec");

    // Should erase full screen: 0..25 rows, 0..80 cols
    ASSERT_TRUE(g_cb.erase_count >= 1);
    ASSERT_EQ(g_cb.erase[0].rect.start_row, 0);
    ASSERT_EQ(g_cb.erase[0].rect.end_row, 25);
    ASSERT_EQ(g_cb.erase[0].rect.start_col, 0);
    ASSERT_EQ(g_cb.erase[0].rect.end_col, 80);
}

// ===== RIS cancels scroll region =====
TEST(seq_ris_cancels_scroll_region)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Set scroll region to rows 5..10 (1-based)
    push(vt, "\e[5;10r");

    // RIS, then move to last row and newline to trigger scroll
    callbacks_clear();
    push(vt, "\ec\e[25H\n");

    // Scroll should cover the full screen 0..25, not just the old region
    ASSERT_EQ(g_cb.scrollrect_count, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_row, 0);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_row, 25);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_col, 0);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_col, 80);
    ASSERT_EQ(g_cb.scrollrect[0].downward, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rightward, 0);
}

// ===== RIS resets modes: cursor visibility and blink restored =====
TEST(seq_ris_resets_modes)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Hide cursor: CSI ? 25 l
    push(vt, "\e[?25l");

    // Verify cursor visibility was turned off
    ASSERT_TRUE(g_cb.settermprop_count >= 1);
    {
        int32_t found = 0;
        int32_t i;
        for (i = 0; i < g_cb.settermprop_count; i++) {
            if (g_cb.settermprop[i].prop == Prop::CursorVisible &&
                g_cb.settermprop[i].val.boolean == false) {
                found = 1;
            }
        }
        ASSERT_TRUE(found);
    }

    // RIS
    callbacks_clear();
    push(vt, "\ec");

        // After RIS, cursor visibility should be restored to true.
    // vterm_state_reset calls settermprop_bool(CURSORVISIBLE, 1).
    {
        int32_t found_visible = 0;
        int32_t i;
        for (i = 0; i < g_cb.settermprop_count; i++) {
            if (g_cb.settermprop[i].prop == Prop::CursorVisible &&
                g_cb.settermprop[i].val.boolean == true) {
                found_visible = 1;
            }
        }
        ASSERT_TRUE(found_visible);
    }

    // Also verify cursor blink is restored to true
    {
        int32_t found_blink = 0;
        int32_t i;
        for (i = 0; i < g_cb.settermprop_count; i++) {
            if (g_cb.settermprop[i].prop == Prop::CursorBlink &&
                g_cb.settermprop[i].val.boolean == true) {
                found_blink = 1;
            }
        }
        ASSERT_TRUE(found_blink);
    }

    // And cursor shape is BLOCK
    {
        int32_t found_shape = 0;
        int32_t i;
        for (i = 0; i < g_cb.settermprop_count; i++) {
            if (g_cb.settermprop[i].prop == Prop::CursorShape &&
                g_cb.settermprop[i].val.number == static_cast<int32_t>(CursorShape::Block)) {
                found_shape = 1;
            }
        }
        ASSERT_TRUE(found_shape);
    }
}

// ===== DECSTR soft reset resets SGR attributes =====
TEST(seq_decstr_soft_reset)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Set bold
    push(vt, "\e[1m");
    ASSERT_PEN_BOOL(state, Attr::Bold, true);

    // Move cursor somewhere
    push(vt, "\e[10;20H");
    ASSERT_CURSOR(state, 9, 19);

    // DECSTR: CSI ! p
    push(vt, "\e[!p");

    // Pen attributes should be reset to defaults
    ASSERT_PEN_BOOL(state, Attr::Bold, false);
    ASSERT_PEN_BOOL(state, Attr::Italic, false);
    ASSERT_PEN_INT(state, Attr::Underline, 0);
    ASSERT_PEN_BOOL(state, Attr::Reverse, false);
    ASSERT_PEN_BOOL(state, Attr::Strike, false);

        // Note: DECSTR does NOT home the cursor or erase the screen.
    // Cursor position may or may not be preserved depending on impl.
    // The key thing is that SGR attributes are reset.
}
