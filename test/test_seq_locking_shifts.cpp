// test_seq_locking_shifts.cpp -- tests for Locking Shifts (SI, SO, LS2, LS3)
//
// SI  (0x0F) selects G0 into GL (Locking Shift 0)
// SO  (0x0E) selects G1 into GL (Locking Shift 1)
// ESC n      selects G2 into GL (Locking Shift 2)
// ESC o      selects G3 into GL (Locking Shift 3)
//
// DEC Special Graphics mappings used for verification:
//   'a' -> U+2592, 'j' -> U+2518, 'l' -> U+250C, 'q' -> U+2500

#include "harness.h"

// ===== SI selects G0 into GL =====
TEST(seq_ls_si_selects_g0)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

        // After reset, G0 is already in GL with ASCII.
    // Send SI (0x0F) to explicitly select G0, then type a char.
    push(vt, "\x0f");
    push(vt, "a");

    ASSERT_EQ(g_cb.putglyph_count, 1);
    // G0 is ASCII by default, so 'a' -> 0x61
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 0x61);
    ASSERT_EQ(g_cb.putglyph[0].width, 1);
}

// ===== SO selects G1 into GL =====
TEST(seq_ls_so_selects_g1)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Designate G1 as DEC Special Graphics: ESC ) 0
    push(vt, "\e)0");

    // Without SO, 'a' should still be ASCII (GL = G0)
    push(vt, "a");
    ASSERT_EQ(g_cb.putglyph_count, 1);
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 0x61);

    callbacks_clear();

    // SO (0x0E) selects G1 into GL
    push(vt, "\x0e");
    push(vt, "a");
    ASSERT_EQ(g_cb.putglyph_count, 1);
    // G1 is DEC graphics, 'a' -> U+2592
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 0x2592);
    ASSERT_EQ(g_cb.putglyph[0].width, 1);

    // Switch back to G0 with SI
    callbacks_clear();
    push(vt, "\x0f");
    push(vt, "a");
    ASSERT_EQ(g_cb.putglyph_count, 1);
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 0x61);
}

// ===== LS2 (ESC n) selects G2 into GL =====
TEST(seq_ls_ls2_selects_g2)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Designate G2 as DEC Special Graphics: ESC * 0
    push(vt, "\e*0");

    // Before LS2, 'a' is still ASCII (GL = G0)
    push(vt, "a");
    ASSERT_EQ(g_cb.putglyph_count, 1);
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 0x61);

    callbacks_clear();

    // LS2: ESC n -- selects G2 into GL
    push(vt, "\en");
    push(vt, "a");
    ASSERT_EQ(g_cb.putglyph_count, 1);
    // G2 is DEC graphics, 'a' -> U+2592
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 0x2592);
    ASSERT_EQ(g_cb.putglyph[0].width, 1);

    // Switch back to G0 with SI
    callbacks_clear();
    push(vt, "\x0f");
    push(vt, "a");
    ASSERT_EQ(g_cb.putglyph_count, 1);
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 0x61);
}

// ===== LS3 (ESC o) selects G3 into GL =====
TEST(seq_ls_ls3_selects_g3)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Designate G3 as DEC Special Graphics: ESC + 0
    push(vt, "\e+0");

    // Before LS3, 'a' is still ASCII (GL = G0)
    push(vt, "a");
    ASSERT_EQ(g_cb.putglyph_count, 1);
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 0x61);

    callbacks_clear();

    // LS3: ESC o -- selects G3 into GL
    push(vt, "\eo");
    push(vt, "a");
    ASSERT_EQ(g_cb.putglyph_count, 1);
    // G3 is DEC graphics, 'a' -> U+2592
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 0x2592);
    ASSERT_EQ(g_cb.putglyph[0].width, 1);

    // Switch back to G0 with SI
    callbacks_clear();
    push(vt, "\x0f");
    push(vt, "a");
    ASSERT_EQ(g_cb.putglyph_count, 1);
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 0x61);
}
