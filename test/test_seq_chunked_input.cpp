// test_seq_chunked_input.cpp -- parser robustness with chunked/split input
//
// Terminal emulators receive input byte by byte or in arbitrary-sized
// chunks. These tests verify that escape sequences split across
// multiple writes are reassembled and processed correctly.

#include "harness.h"

// ============================================================================
// CSI split after ESC
// ============================================================================

// Send "\e" then "[A" as two writes — cursor should move up
TEST(seq_chunked_csi_split_after_esc)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Move cursor to row 5 first
    push(vt, "\e[6;1H"); // CUP row 6, col 1 (1-based) = row 5, col 0
    ASSERT_CURSOR(state, 5, 0);

    // Split CSI A (cursor up) across two writes
    push(vt, {"\e", 1});
    push(vt, {"[A", 2});
    ASSERT_CURSOR(state, 4, 0);
}

// ============================================================================
// CSI split in the middle of parameters
// ============================================================================

// Send "\e[1" then "0;20H" — cursor should be at (9, 19)
TEST(seq_chunked_csi_split_in_params)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Split CUP 10;20 H across two writes in the middle of the first param
    push(vt, {"\e[1", 3});
    push(vt, {"0;20H", 5});
    ASSERT_CURSOR(state, 9, 19);
}

// ============================================================================
// CSI sent one byte at a time
// ============================================================================

// Send "\e[5;5H" (6 bytes) one byte at a time
TEST(seq_chunked_csi_single_byte)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Send each byte individually
    push(vt, {"\e", 1});
    push(vt, {"[", 1});
    push(vt, {"5", 1});
    push(vt, {";", 1});
    push(vt, {"5", 1});
    push(vt, {"H", 1});
    ASSERT_CURSOR(state, 4, 4);
}

// ============================================================================
// OSC split between data and terminator
// ============================================================================

// Send OSC title in two parts: "\e]2;Hello" then "\e\\"
TEST(seq_chunked_osc_split)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // First part: OSC 2 with data, not yet terminated
    push(vt, "\e]2;Hello");

    // The initial fragment should have been received
    ASSERT_TRUE(g_cb.settermprop_count >= 1);
    ASSERT_EQ(g_cb.settermprop[0].prop, Prop::Title);
    ASSERT_TRUE(g_cb.settermprop[0].val.string.initial);
    // Not yet final
    ASSERT_TRUE(!g_cb.settermprop[g_cb.settermprop_count - 1].val.string.final_);

    // Second part: ST terminator
    int32_t prev_count = g_cb.settermprop_count;
    push(vt, "\e\\");

    // Now the final fragment should have arrived
    ASSERT_TRUE(g_cb.settermprop_count > prev_count);
    ASSERT_TRUE(g_cb.settermprop[g_cb.settermprop_count - 1].val.string.final_);
}

// ============================================================================
// UTF-8 character split across writes
// ============================================================================

// Send a 2-byte UTF-8 char (U+00E9 = e-acute = 0xC3 0xA9) in two writes
TEST(seq_chunked_utf8_split)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    Screen& screen = vt.screen();
    screen.enable_altscreen(true);
    screen.set_callbacks(screen_cbs);
    screen.reset(true);
    callbacks_clear();

    // Send the two bytes of U+00E9 (e-acute) separately
    push(vt, {"\xC3", 1});
    push(vt, {"\xA9", 1});

    // Verify the character appears at row 0, col 0
    {
        Pos pos = { .row = 0, .col = 0 };
        ScreenCell cell;
        (void)screen.get_cell(pos, cell);
        ASSERT_EQ(cell.chars[0], 0x00E9);
        ASSERT_EQ(cell.width, 1);
    }
}
