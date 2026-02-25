// test_seq_decsc_decrc.cpp -- tests for DECSC (ESC 7) / DECRC (ESC 8) Save/Restore Cursor
//
// DECSC saves the cursor position and pen attributes.
// DECRC restores them.
// Mode 1048 (CSI ? 1048 h / l) provides an alternative way to
// save/restore that behaves identically.

#include "harness.h"

// ===== DECSC saves and DECRC restores cursor position =====
TEST(seq_decsc_saves_position)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Move cursor to (5, 10) -- 1-based: 6;11
    push(vt, "\e[6;11H");
    ASSERT_CURSOR(state, 5, 10);

    // Save cursor: ESC 7
    push(vt, "\e7");

    // Move cursor away to (0, 0)
    push(vt, "\e[H");
    ASSERT_CURSOR(state, 0, 0);

    // Restore cursor: ESC 8
    push(vt, "\e8");
    ASSERT_CURSOR(state, 5, 10);
}

// ===== DECSC saves and DECRC restores pen attributes =====
TEST(seq_decsc_saves_pen)
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

    // Save cursor + pen
    push(vt, "\e7");

    // Reset SGR
    push(vt, "\e[0m");
    ASSERT_PEN_BOOL(state, Attr::Bold, false);

    // Restore -- bold should come back
    push(vt, "\e8");
    ASSERT_PEN_BOOL(state, Attr::Bold, true);
}

// ===== DECSC/DECRC roundtrip with multiple attributes =====
TEST(seq_decsc_roundtrip)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Set up: position (3, 7), bold, underline
    push(vt, "\e[4;8H");
    ASSERT_CURSOR(state, 3, 7);
    push(vt, "\e[1;4m");
    ASSERT_PEN_BOOL(state, Attr::Bold, true);
    ASSERT_PEN_INT(state, Attr::Underline, 1);

    // Save
    push(vt, "\e7");

    // Change everything: move cursor, reset SGR, set italic
    push(vt, "\e[15;40H");
    push(vt, "\e[0;3m");
    ASSERT_CURSOR(state, 14, 39);
    ASSERT_PEN_BOOL(state, Attr::Bold, false);
    ASSERT_PEN_INT(state, Attr::Underline, 0);
    ASSERT_PEN_BOOL(state, Attr::Italic, true);

    // Restore
    push(vt, "\e8");
    ASSERT_CURSOR(state, 3, 7);
    ASSERT_PEN_BOOL(state, Attr::Bold, true);
    ASSERT_PEN_INT(state, Attr::Underline, 1);
    ASSERT_PEN_BOOL(state, Attr::Italic, false);
}

// ===== Mode 1048 save/restore behaves like DECSC/DECRC =====
TEST(seq_decsc_via_mode_1048)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Move cursor to (7, 15) -- 1-based: 8;16
    push(vt, "\e[8;16H");
    ASSERT_CURSOR(state, 7, 15);

    // Save via mode 1048: CSI ? 1048 h
    push(vt, "\e[?1048h");

    // Move cursor away
    push(vt, "\e[1;1H");
    ASSERT_CURSOR(state, 0, 0);

    // Restore via mode 1048: CSI ? 1048 l
    push(vt, "\e[?1048l");
    ASSERT_CURSOR(state, 7, 15);
}
