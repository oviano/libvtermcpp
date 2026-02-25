// test_seq_decset_altscreen.cpp -- tests for altscreen and related DECSET modes
//
// Mode 1047: Alternate screen buffer
// Mode 1049: Alternate screen buffer with save/restore cursor
// Mode 2004: Bracketed paste mode
// Mode 1004: Focus event reporting

#include "harness.h"

// ===== Mode 1047: switch to altscreen =====
TEST(seq_decset_altscreen_1047_switch)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    Screen& screen = vt.screen();
    screen.enable_altscreen(true);
    screen.set_callbacks(screen_cbs);
    screen.reset(true);
    callbacks_clear();

    // Enable altscreen via mode 1047
    push(vt, "\e[?1047h");

    // Look for Prop::AltScreen = true in settermprop callbacks
    int32_t found = 0;
    for (int32_t i = 0; i < g_cb.settermprop_count; i++) {
        if (g_cb.settermprop[i].prop == Prop::AltScreen &&
            g_cb.settermprop[i].val.boolean == true) {
            found = 1;
            break;
        }
    }
    ASSERT_TRUE(found);
}

// ===== Mode 1047: restore from altscreen =====
TEST(seq_decset_altscreen_1047_restore)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    Screen& screen = vt.screen();
    screen.enable_altscreen(true);
    screen.set_callbacks(screen_cbs);
    screen.reset(true);
    callbacks_clear();

    // Switch to altscreen then back
    push(vt, "\e[?1047h");
    callbacks_clear();

    push(vt, "\e[?1047l");

    // Look for Prop::AltScreen = false
    int32_t found = 0;
    for (int32_t i = 0; i < g_cb.settermprop_count; i++) {
        if (g_cb.settermprop[i].prop == Prop::AltScreen &&
            g_cb.settermprop[i].val.boolean == false) {
            found = 1;
            break;
        }
    }
    ASSERT_TRUE(found);
}

// ===== Mode 1049: combined save cursor + altscreen =====
TEST(seq_decset_altscreen_1049_combined)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    Screen& screen = vt.screen();
    screen.enable_altscreen(true);
    screen.set_callbacks(screen_cbs);
    screen.reset(true);

    // Write something on main screen, then move cursor to a known position
    push(vt, "Main");
    // Cursor is now at row 0, col 4 on main screen

    // Move to row 5, col 10 (1-based)
    push(vt, "\e[5;10H");

    callbacks_clear();

    // Enable mode 1049: saves cursor + switches to altscreen
    push(vt, "\e[?1049h");

    // Should fire Prop::AltScreen = true
    int32_t found_on = 0;
    for (int32_t i = 0; i < g_cb.settermprop_count; i++) {
        if (g_cb.settermprop[i].prop == Prop::AltScreen &&
            g_cb.settermprop[i].val.boolean == true) {
            found_on = 1;
            break;
        }
    }
    ASSERT_TRUE(found_on);

    // Altscreen should be clean -- first row empty
    ASSERT_SCREEN_ROW(vt, screen, 0, "");

    // Move cursor on altscreen
    push(vt, "\e[1;1H");
    push(vt, "Alt");

    callbacks_clear();

    // Disable mode 1049: restores cursor + switches back to main
    push(vt, "\e[?1049l");

    // Should fire Prop::AltScreen = false
    int32_t found_off = 0;
    for (int32_t i = 0; i < g_cb.settermprop_count; i++) {
        if (g_cb.settermprop[i].prop == Prop::AltScreen &&
            g_cb.settermprop[i].val.boolean == false) {
            found_off = 1;
            break;
        }
    }
    ASSERT_TRUE(found_off);

    // Main screen should still have "Main"
    ASSERT_SCREEN_ROW(vt, screen, 0, "Main");
}

// ===== Mode 2004: bracketed paste mode =====
TEST(seq_decset_bracketed_paste_2004)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    output_init(vt);
    state.reset(true);
    callbacks_clear();

    // Enable bracketed paste mode
    push(vt, "\e[?2004h");

    // Start paste -- should output \e[200~
    output_clear();
    vt.keyboard_start_paste();
    ASSERT_OUTPUT_BYTES("\e[200~", 6);

    // End paste -- should output \e[201~
    output_clear();
    vt.keyboard_end_paste();
    ASSERT_OUTPUT_BYTES("\e[201~", 6);
}

// ===== Mode 1004: focus event reporting =====
TEST(seq_decset_focus_report_1004)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    output_init(vt);
    state.reset(true);
    callbacks_clear();

    // Enable focus reporting
    push(vt, "\e[?1004h");

    // Verify settermprop FOCUSREPORT = true
    ASSERT_EQ(g_cb.settermprop_count, 1);
    ASSERT_EQ(g_cb.settermprop[0].prop, Prop::FocusReport);
    ASSERT_EQ(g_cb.settermprop[0].val.boolean, true);

    // Focus in should output \e[I
    output_clear();
    state.focus_in();
    ASSERT_OUTPUT_BYTES("\e[I", 3);

    // Focus out should output \e[O
    output_clear();
    state.focus_out();
    ASSERT_OUTPUT_BYTES("\e[O", 3);
}
