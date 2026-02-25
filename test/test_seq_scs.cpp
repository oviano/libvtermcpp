// test_seq_scs.cpp -- tests for SCS (Select Character Set) G0-G3
//
// SCS designates a character set into one of the G-sets (G0..G3):
//   ESC ( X  -> designate G0
//   ESC ) X  -> designate G1
//   ESC * X  -> designate G2
//   ESC + X  -> designate G3
//
// Where X is the charset designator:
//   B -> US-ASCII
//   0 -> DEC Special Graphics
//   A -> UK
//
// DEC Special Graphics character mappings used here:
//   'l' -> U+250C (box drawings light down and right)
//   'j' -> U+2518 (box drawings light up and left)
//   'k' -> U+2510 (box drawings light down and left)
//   'a' -> U+2592 (medium shade)
//   'q' -> U+2500 (box drawings light horizontal)

#include "harness.h"

// ===== G0 designated as DEC Special Graphics =====
TEST(seq_scs_g0_dec_graphics)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Designate G0 as DEC Special Graphics: ESC ( 0
    push(vt, "\e(0");
    push(vt, "l");

    ASSERT_EQ(g_cb.putglyph_count, 1);
    // 'l' in DEC graphics -> U+250C
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 0x250C);
    ASSERT_EQ(g_cb.putglyph[0].width, 1);
    ASSERT_EQ(g_cb.putglyph[0].row, 0);
    ASSERT_EQ(g_cb.putglyph[0].col, 0);
}

// ===== G0 designated as ASCII =====
TEST(seq_scs_g0_ascii)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // First set G0 to DEC graphics, then back to ASCII
    push(vt, "\e(0");
    push(vt, "\e(B");
    push(vt, "l");

    ASSERT_EQ(g_cb.putglyph_count, 1);
    // 'l' in ASCII -> 0x6C
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 0x6C);
    ASSERT_EQ(g_cb.putglyph[0].width, 1);
    ASSERT_EQ(g_cb.putglyph[0].row, 0);
    ASSERT_EQ(g_cb.putglyph[0].col, 0);
}

// ===== G1 designated as DEC graphics, shifted via SO =====
TEST(seq_scs_g1_dec_graphics)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Designate G1 as DEC Special Graphics: ESC ) 0
    push(vt, "\e)0");

    // Switch to G1 via SO (0x0E)
    push(vt, "\x0e");
    push(vt, "l");

    ASSERT_EQ(g_cb.putglyph_count, 1);
    // 'l' in DEC graphics -> U+250C
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 0x250C);
    ASSERT_EQ(g_cb.putglyph[0].width, 1);
    ASSERT_EQ(g_cb.putglyph[0].row, 0);
    ASSERT_EQ(g_cb.putglyph[0].col, 0);
}

// ===== G1 DEC graphics with shift and SI back =====
TEST(seq_scs_with_shift)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Designate G1 as DEC Special Graphics
    push(vt, "\e)0");

    // SO to switch to G1
    push(vt, "\x0e");
    push(vt, "j");

    ASSERT_EQ(g_cb.putglyph_count, 1);
    // 'j' in DEC graphics -> U+2518
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 0x2518);
    ASSERT_EQ(g_cb.putglyph[0].width, 1);

    callbacks_clear();

    // SI to switch back to G0 (ASCII)
    push(vt, "\x0f");
    push(vt, "j");

    ASSERT_EQ(g_cb.putglyph_count, 1);
    // 'j' in ASCII -> 0x6A
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 0x6A);
    ASSERT_EQ(g_cb.putglyph[0].width, 1);
}
