// test_seq_s7c1t_s8c1t.cpp -- per-sequence tests for S7C1T / S8C1T
//
// S7C1T (ESC SP F) -- send 7-bit C1 controls (default)
// S8C1T (ESC SP G) -- send 8-bit C1 controls
//
// When S8C1T is active, output uses single-byte C1 codes (0x80-0x9F)
// instead of the 7-bit ESC+letter equivalents. For example, CSI becomes
// 0x9B instead of ESC [.

#include "harness.h"

// ============================================================================
// Default 7-bit output
// ============================================================================

// By default (S7C1T), DSR response uses 7-bit CSI (\e[)
TEST(seq_s7c1t_7bit_output)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    output_init(vt);
    state.reset(true);
    callbacks_clear();
    output_clear();

    // DSR operating status -> CSI 0 n in 7-bit form
    push(vt, "\e[5n");
    ASSERT_OUTPUT_BYTES("\e[0n", 4);
}

// ============================================================================
// S8C1T -- enable 8-bit C1 output
// ============================================================================

// After ESC SP G (S8C1T), DSR response uses 8-bit CSI (0x9B)
TEST(seq_s8c1t_8bit_output)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    output_init(vt);
    state.reset(true);
    callbacks_clear();
    output_clear();

    // Enable S8C1T
    push(vt, "\e G");
    output_clear();

    // DSR operating status -> 0x9B '0' 'n' (3 bytes)
    push(vt, "\e[5n");
    ASSERT_OUTPUT_BYTES("\x9b" "0n", 3);
}

// ============================================================================
// S7C1T -- restore 7-bit output after S8C1T
// ============================================================================

// After enabling S8C1T then S7C1T, output returns to 7-bit
TEST(seq_s7c1t_restore_7bit)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    output_init(vt);
    state.reset(true);
    callbacks_clear();
    output_clear();

    // Enable S8C1T
    push(vt, "\e G");
    output_clear();

    // Verify 8-bit mode is active
    push(vt, "\e[5n");
    ASSERT_OUTPUT_BYTES("\x9b" "0n", 3);

    // Restore S7C1T
    push(vt, "\e F");
    output_clear();

    // DSR should now use 7-bit CSI again
    push(vt, "\e[5n");
    ASSERT_OUTPUT_BYTES("\e[0n", 4);
}

// ============================================================================
// S8C1T -- DA primary (CSI response)
// ============================================================================

// DA primary in 8-bit mode: CSI becomes 0x9B
TEST(seq_s8c1t_da_primary)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    output_init(vt);
    state.reset(true);
    callbacks_clear();
    output_clear();

    push(vt, "\e G");
    output_clear();

        // DA primary: 7-bit would be \e[?1;2c (7 bytes)
    // 8-bit: 0x9B ?1;2c (6 bytes)
    push(vt, "\e[c");
    ASSERT_OUTPUT_BYTES("\x9b" "?1;2c", 6);
}

// ============================================================================
// S8C1T -- DA secondary (CSI response)
// ============================================================================

// DA secondary in 8-bit mode
TEST(seq_s8c1t_da_secondary)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    output_init(vt);
    state.reset(true);
    callbacks_clear();
    output_clear();

    push(vt, "\e G");
    output_clear();

        // DA2: 7-bit would be \e[>0;100;0c (11 bytes)
    // 8-bit: 0x9B >0;100;0c (10 bytes)
    push(vt, "\e[>c");
    ASSERT_OUTPUT_BYTES("\x9b" ">0;100;0c", 10);
}

// ============================================================================
// S8C1T -- DECRQM (CSI response)
// ============================================================================

// DECRQM in 8-bit mode
TEST(seq_s8c1t_decrqm)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    output_init(vt);
    state.reset(true);
    callbacks_clear();
    output_clear();

    push(vt, "\e G");
    output_clear();

        // DECRQM for mouse mode 1000 (off=2): 7-bit \e[?1000;2$y (11 bytes)
    // 8-bit: 0x9B ?1000;2$y (10 bytes)
    push(vt, "\e[?1000$p");
    ASSERT_OUTPUT_BYTES("\x9b" "?1000;2$y", 10);
}

// ============================================================================
// S8C1T -- DECRQSS (DCS+ST response)
// ============================================================================

// DECRQSS in 8-bit mode: DCS becomes 0x90, ST becomes 0x9C
TEST(seq_s8c1t_decrqss)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    output_init(vt);
    state.reset(true);
    callbacks_clear();
    output_clear();

    push(vt, "\e G");
    output_clear();

    // Set bold
    push(vt, "\e[1m");
    output_clear();

        // DECRQSS for SGR: 7-bit \eP1$r1m\e\\ (9 bytes)
    // 8-bit: 0x90 1$r1m 0x9C (7 bytes)
    push(vt, "\eP$qm\e\\");
    ASSERT_OUTPUT_BYTES("\x90" "1$r1m" "\x9c", 7);
}

// ============================================================================
// S8C1T -- XTVERSION (DCS+ST response)
// ============================================================================

// XTVERSION in 8-bit mode
TEST(seq_s8c1t_xtversion)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    output_init(vt);
    state.reset(true);
    callbacks_clear();
    output_clear();

    push(vt, "\e G");
    output_clear();

        // XTVERSION: 7-bit \eP>|libvterm(0.3)\e\\ (19 bytes)
    // 8-bit: 0x90 >|libvterm(0.3) 0x9C (17 bytes)
    push(vt, "\e[>q");
    ASSERT_OUTPUT_BYTES("\x90" ">|libvterm(0.3)" "\x9c", 17);
}

// ============================================================================
// S8C1T -- keyboard arrow key (CSI response)
// ============================================================================

// Arrow key output in 8-bit mode
TEST(seq_s8c1t_keyboard_arrow)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    output_init(vt);
    state.reset(true);
    callbacks_clear();
    output_clear();

    push(vt, "\e G");
    output_clear();

    // Up arrow: 7-bit \e[A (3 bytes), 8-bit: 0x9B A (2 bytes)
    vt.keyboard_key(Key::Up, Modifier::None);
    ASSERT_OUTPUT_BYTES("\x9b" "A", 2);
}

// ============================================================================
// S8C1T -- keyboard F1 (SS3 response)
// ============================================================================

// F1 key output in 8-bit mode: SS3 becomes 0x8F
TEST(seq_s8c1t_keyboard_f1)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    output_init(vt);
    state.reset(true);
    callbacks_clear();
    output_clear();

    push(vt, "\e G");
    output_clear();

    // F1: 7-bit \eOP (3 bytes), 8-bit: 0x8F P (2 bytes)
    vt.keyboard_key(static_cast<Key>(static_cast<int32_t>(Key::Function0) + 1), Modifier::None);
    ASSERT_OUTPUT_BYTES("\x8f" "P", 2);
}

// ============================================================================
// S8C1T -- mouse click (CSI response)
// ============================================================================

// Mouse output in 8-bit mode
TEST(seq_s8c1t_mouse)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    output_init(vt);
    state.reset(true);
    callbacks_clear();
    output_clear();

    // Enable mouse click mode
    push(vt, "\e[?1000h");
    // Enable S8C1T
    push(vt, "\e G");
    callbacks_clear();
    output_clear();

    vt.mouse_move(0, 0, Modifier::None);
    vt.mouse_button(1, 1, Modifier::None);
        // 7-bit: \e[M <button> <col> <row> (6 bytes)
    // 8-bit: 0x9B M <button> <col> <row> (5 bytes)
    ASSERT_OUTPUT_BYTES("\x9b" "M\x20\x21\x21", 5);
}

// ============================================================================
// S8C1T -- SGR mouse (CSI response)
// ============================================================================

// SGR extended mouse output in 8-bit mode
TEST(seq_s8c1t_mouse_sgr)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    output_init(vt);
    state.reset(true);
    callbacks_clear();
    output_clear();

    // Enable mouse + SGR encoding
    push(vt, "\e[?1003;1006h");
    push(vt, "\e G");
    callbacks_clear();
    output_clear();

    vt.mouse_move(5, 10, Modifier::None);
    output_clear();
    vt.mouse_button(1, 1, Modifier::None);
        // 7-bit: \e[<0;11;6M (10 bytes)
    // 8-bit: 0x9B <0;11;6M (9 bytes)
    ASSERT_OUTPUT_BYTES("\x9b" "<0;11;6M", 9);
}

// ============================================================================
// S8C1T -- focus in/out (CSI response)
// ============================================================================

// Focus events in 8-bit mode
TEST(seq_s8c1t_focus)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    output_init(vt);
    state.reset(true);
    callbacks_clear();
    output_clear();

    // Enable focus reporting
    push(vt, "\e[?1004h");
    push(vt, "\e G");
    callbacks_clear();
    output_clear();

    // Focus in: 7-bit \e[I (3 bytes), 8-bit: 0x9B I (2 bytes)
    state.focus_in();
    ASSERT_OUTPUT_BYTES("\x9b" "I", 2);

    // Focus out: 7-bit \e[O (3 bytes), 8-bit: 0x9B O (2 bytes)
    state.focus_out();
    ASSERT_OUTPUT_BYTES("\x9b" "O", 2);
}

// ============================================================================
// S8C1T -- bracket paste (CSI response)
// ============================================================================

// Bracket paste markers in 8-bit mode
TEST(seq_s8c1t_bracket_paste)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    output_init(vt);
    state.reset(true);
    callbacks_clear();
    output_clear();

    // Enable bracket paste mode
    push(vt, "\e[?2004h");
    push(vt, "\e G");
    output_clear();

    // Start paste: 7-bit \e[200~ (6 bytes), 8-bit: 0x9B 200~ (5 bytes)
    vt.keyboard_start_paste();
    ASSERT_OUTPUT_BYTES("\x9b" "200~", 5);

    // End paste: 7-bit \e[201~ (6 bytes), 8-bit: 0x9B 201~ (5 bytes)
    vt.keyboard_end_paste();
    ASSERT_OUTPUT_BYTES("\x9b" "201~", 5);
}

// ============================================================================
// S8C1T -- cursor position report (DSR 6n)
// ============================================================================

// Extended cursor position report in 8-bit mode
TEST(seq_s8c1t_dsr_cursor)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    output_init(vt);
    state.reset(true);
    callbacks_clear();
    output_clear();

    push(vt, "\e G");
    output_clear();

    // Move cursor to row 5, col 10
    push(vt, "\e[5;10H");
    output_clear();

        // DSR 6 (cursor position): 7-bit \e[5;10R (7 bytes)
    // 8-bit: 0x9B 5;10R (6 bytes)
    push(vt, "\e[6n");
    ASSERT_OUTPUT_BYTES("\x9b" "5;10R", 6);
}

// ============================================================================
// S8C1T -- keyboard function key with modifier (CSI response)
// ============================================================================

// Function key with modifier in 8-bit mode
TEST(seq_s8c1t_keyboard_f5_shift)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    output_init(vt);
    state.reset(true);
    callbacks_clear();
    output_clear();

    push(vt, "\e G");
    output_clear();

    // Shift+F5: 7-bit \e[15;2~ (7 bytes), 8-bit: 0x9B 15;2~ (6 bytes)
    vt.keyboard_key(static_cast<Key>(static_cast<int32_t>(Key::Function0) + 5), Modifier::Shift);
    ASSERT_OUTPUT_BYTES("\x9b" "15;2~", 6);
}
