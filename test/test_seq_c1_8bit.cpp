// test_seq_c1_8bit.cpp -- tests for 8-bit C1 control codes (0x80-0x9F)
//
// When vt.set_utf8(false) (non-UTF-8 mode), bytes 0x80-0x9F are
// interpreted as C1 control codes:
//   0x84 = IND  (Index)
//   0x85 = NEL  (Next Line)
//   0x88 = HTS  (Horizontal Tab Stop)
//   0x8D = RI   (Reverse Index)
//   0x90 = DCS  (Device Control String)
//   0x9B = CSI  (Control Sequence Introducer)
//   0x9C = ST   (String Terminator)
//   0x9D = OSC  (Operating System Command)
//
// In UTF-8 mode, these bytes are NOT treated as C1 controls — they are
// part of UTF-8 multibyte sequences.

#include "harness.h"

// ============================================================================
// 0x9B — 8-bit CSI
// ============================================================================

// 0x9B followed by "5;5H" should position cursor at (4, 4)
TEST(seq_c1_csi_0x9b)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    push(vt, {"\x9b" "5;5H", 5});
    ASSERT_CURSOR(state, 4, 4);
}

// ============================================================================
// 0x84 — 8-bit IND (Index)
// ============================================================================

// 0x84 moves cursor down one row, same as ESC D
TEST(seq_c1_ind_0x84)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Start at row 0
    ASSERT_CURSOR(state, 0, 0);
    push(vt, {"\x84", 1});
    ASSERT_CURSOR(state, 1, 0);
}

// ============================================================================
// 0x85 — 8-bit NEL (Next Line)
// ============================================================================

// 0x85 moves to the start of the next line
TEST(seq_c1_nel_0x85)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Move to col 10 first
    push(vt, "\e[11G"); // CHA 11 = col 10 (0-based)
    ASSERT_CURSOR(state, 0, 10);

    push(vt, {"\x85", 1});
    // NEL: row 1, col 0
    ASSERT_CURSOR(state, 1, 0);
}

// ============================================================================
// 0x88 — 8-bit HTS (Horizontal Tab Stop)
// ============================================================================

// 0x88 sets a tab stop at the current column
TEST(seq_c1_hts_0x88)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Clear all tab stops
    push(vt, "\e[3g");

    // Move to col 5, set HTS via 8-bit C1
    push(vt, "\e[6G"); // CHA 6 = col 5 (0-based)
    ASSERT_CURSOR(state, 0, 5);
    push(vt, {"\x88", 1});

    // Go back to col 0, tab should go to col 5
    push(vt, "\e[1G");
    ASSERT_CURSOR(state, 0, 0);
    push(vt, "\t");
    ASSERT_CURSOR(state, 0, 5);
}

// ============================================================================
// 0x8D — 8-bit RI (Reverse Index)
// ============================================================================

// 0x8D moves cursor up one row, same as ESC M
TEST(seq_c1_ri_0x8d)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Move to row 5
    push(vt, "\e[6;1H"); // CUP row 6, col 1 (1-based) = row 5, col 0
    ASSERT_CURSOR(state, 5, 0);

    push(vt, {"\x8d", 1});
    ASSERT_CURSOR(state, 4, 0);
}

// ============================================================================
// 0x9D — 8-bit OSC, terminated by 0x9C (8-bit ST)
// ============================================================================

// 8-bit OSC to set title, terminated by 8-bit ST
TEST(seq_c1_osc_0x9d)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // 0x9D = OSC, "2;TestTitle", 0x9C = ST
    push(vt, {"\x9d" "2;TestTitle\x9c", 13});

    // Verify title was set
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
// 0x90 — 8-bit DCS for DECRQSS SGR query
// ============================================================================

// 8-bit DCS $q m with 8-bit ST to query SGR
TEST(seq_c1_dcs_0x90)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    output_init(vt);
    state.reset(true);
    callbacks_clear();
    output_clear();

    // Set bold
    push(vt, "\e[1m");
    output_clear();

    // Query SGR via 8-bit DCS: 0x90 "$qm" 0x9C
    push(vt, {"\x90" "$qm\x9c", 5});

        // Response should be a DCS with SGR value.
    // Expected: \eP1$r1m\e\\ (9 bytes)
    ASSERT_OUTPUT_BYTES("\eP1$r1m\e\\", 9);
}
