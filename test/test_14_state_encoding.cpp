// test_14_state_encoding.cpp -- state encoding tests
// Ported from upstream libvterm t/14state_encoding.test

#include "harness.h"

// Default
TEST(state_encoding_default)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);

    state.reset(true);
    callbacks_clear();
    push(vt, "#");
    ASSERT_EQ(g_cb.putglyph_count, 1);
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 0x23);
    ASSERT_EQ(g_cb.putglyph[0].width, 1);
    ASSERT_EQ(g_cb.putglyph[0].row, 0);
    ASSERT_EQ(g_cb.putglyph[0].col, 0);
}

// Designate G0=UK
TEST(state_encoding_designate_g0_uk)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);

    state.reset(true);
    callbacks_clear();
    push(vt, "\e(A");
    push(vt, "#");
    ASSERT_EQ(g_cb.putglyph_count, 1);
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 0x00a3);
    ASSERT_EQ(g_cb.putglyph[0].width, 1);
    ASSERT_EQ(g_cb.putglyph[0].row, 0);
    ASSERT_EQ(g_cb.putglyph[0].col, 0);
}

// Designate G0=DEC drawing
TEST(state_encoding_designate_g0_dec_drawing)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);

    state.reset(true);
    callbacks_clear();
    push(vt, "\e(0");
    push(vt, "a");
    ASSERT_EQ(g_cb.putglyph_count, 1);
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 0x2592);
    ASSERT_EQ(g_cb.putglyph[0].width, 1);
    ASSERT_EQ(g_cb.putglyph[0].row, 0);
    ASSERT_EQ(g_cb.putglyph[0].col, 0);
}

// Designate G1 + LS1
TEST(state_encoding_designate_g1_ls1)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);

    state.reset(true);
    callbacks_clear();
    push(vt, "\e)0");
    push(vt, "a");
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 0x61);
    ASSERT_EQ(g_cb.putglyph[0].width, 1);
    ASSERT_EQ(g_cb.putglyph[0].row, 0);
    ASSERT_EQ(g_cb.putglyph[0].col, 0);

    callbacks_clear();
    push(vt, "\x0e");
    push(vt, "a");
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 0x2592);
    ASSERT_EQ(g_cb.putglyph[0].width, 1);
    ASSERT_EQ(g_cb.putglyph[0].row, 0);
    ASSERT_EQ(g_cb.putglyph[0].col, 1);

    // LS0
    callbacks_clear();
    push(vt, "\x0f");
    push(vt, "a");
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 0x61);
    ASSERT_EQ(g_cb.putglyph[0].width, 1);
    ASSERT_EQ(g_cb.putglyph[0].row, 0);
    ASSERT_EQ(g_cb.putglyph[0].col, 2);
}

// Designate G2 + LS2
TEST(state_encoding_designate_g2_ls2)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);

    state.reset(true);
    // Set up prior state: G1=DEC drawing, push some chars to advance cursor
    push(vt, "\e)0");
    push(vt, "a");
    push(vt, "\x0e");
    push(vt, "a");
    push(vt, "\x0f");
    push(vt, "a");

    callbacks_clear();
    push(vt, "\e*0");
    push(vt, "a");
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 0x61);
    ASSERT_EQ(g_cb.putglyph[0].width, 1);
    ASSERT_EQ(g_cb.putglyph[0].row, 0);
    ASSERT_EQ(g_cb.putglyph[0].col, 3);

    callbacks_clear();
    push(vt, "\en");
    push(vt, "a");
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 0x2592);
    ASSERT_EQ(g_cb.putglyph[0].width, 1);
    ASSERT_EQ(g_cb.putglyph[0].row, 0);
    ASSERT_EQ(g_cb.putglyph[0].col, 4);

    callbacks_clear();
    push(vt, "\x0f");
    push(vt, "a");
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 0x61);
    ASSERT_EQ(g_cb.putglyph[0].width, 1);
    ASSERT_EQ(g_cb.putglyph[0].row, 0);
    ASSERT_EQ(g_cb.putglyph[0].col, 5);
}

// Designate G3 + LS3
TEST(state_encoding_designate_g3_ls3)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);

    state.reset(true);
    // Set up prior state from G1+LS1 and G2+LS2 tests
    push(vt, "\e)0");
    push(vt, "a");
    push(vt, "\x0e");
    push(vt, "a");
    push(vt, "\x0f");
    push(vt, "a");
    push(vt, "\e*0");
    push(vt, "a");
    push(vt, "\en");
    push(vt, "a");
    push(vt, "\x0f");
    push(vt, "a");

    callbacks_clear();
    push(vt, "\e+0");
    push(vt, "a");
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 0x61);
    ASSERT_EQ(g_cb.putglyph[0].width, 1);
    ASSERT_EQ(g_cb.putglyph[0].row, 0);
    ASSERT_EQ(g_cb.putglyph[0].col, 6);

    callbacks_clear();
    push(vt, "\eo");
    push(vt, "a");
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 0x2592);
    ASSERT_EQ(g_cb.putglyph[0].width, 1);
    ASSERT_EQ(g_cb.putglyph[0].row, 0);
    ASSERT_EQ(g_cb.putglyph[0].col, 7);

    callbacks_clear();
    push(vt, "\x0f");
    push(vt, "a");
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 0x61);
    ASSERT_EQ(g_cb.putglyph[0].width, 1);
    ASSERT_EQ(g_cb.putglyph[0].row, 0);
    ASSERT_EQ(g_cb.putglyph[0].col, 8);
}

// SS2
TEST(state_encoding_ss2)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);

    state.reset(true);
    // Set up prior state: G2=DEC drawing, cursor at col 9
    push(vt, "\e)0");
    push(vt, "a");
    push(vt, "\x0e");
    push(vt, "a");
    push(vt, "\x0f");
    push(vt, "a");
    push(vt, "\e*0");
    push(vt, "a");
    push(vt, "\en");
    push(vt, "a");
    push(vt, "\x0f");
    push(vt, "a");
    push(vt, "\e+0");
    push(vt, "a");
    push(vt, "\eo");
    push(vt, "a");
    push(vt, "\x0f");
    push(vt, "a");

    callbacks_clear();
    push(vt, {"a\x8e" "aa", 4});
    ASSERT_EQ(g_cb.putglyph_count, 3);
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 0x61);
    ASSERT_EQ(g_cb.putglyph[0].width, 1);
    ASSERT_EQ(g_cb.putglyph[0].row, 0);
    ASSERT_EQ(g_cb.putglyph[0].col, 9);
    ASSERT_EQ(g_cb.putglyph[1].chars[0], 0x2592);
    ASSERT_EQ(g_cb.putglyph[1].width, 1);
    ASSERT_EQ(g_cb.putglyph[1].row, 0);
    ASSERT_EQ(g_cb.putglyph[1].col, 10);
    ASSERT_EQ(g_cb.putglyph[2].chars[0], 0x61);
    ASSERT_EQ(g_cb.putglyph[2].width, 1);
    ASSERT_EQ(g_cb.putglyph[2].row, 0);
    ASSERT_EQ(g_cb.putglyph[2].col, 11);
}

// SS3
TEST(state_encoding_ss3)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);

    state.reset(true);
    // Set up prior state: G3=DEC drawing, cursor at col 12
    push(vt, "\e)0");
    push(vt, "a");
    push(vt, "\x0e");
    push(vt, "a");
    push(vt, "\x0f");
    push(vt, "a");
    push(vt, "\e*0");
    push(vt, "a");
    push(vt, "\en");
    push(vt, "a");
    push(vt, "\x0f");
    push(vt, "a");
    push(vt, "\e+0");
    push(vt, "a");
    push(vt, "\eo");
    push(vt, "a");
    push(vt, "\x0f");
    push(vt, "a");
    push(vt, {"a\x8e" "aa", 4});

    callbacks_clear();
    push(vt, {"a\x8f" "aa", 4});
    ASSERT_EQ(g_cb.putglyph_count, 3);
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 0x61);
    ASSERT_EQ(g_cb.putglyph[0].width, 1);
    ASSERT_EQ(g_cb.putglyph[0].row, 0);
    ASSERT_EQ(g_cb.putglyph[0].col, 12);
    ASSERT_EQ(g_cb.putglyph[1].chars[0], 0x2592);
    ASSERT_EQ(g_cb.putglyph[1].width, 1);
    ASSERT_EQ(g_cb.putglyph[1].row, 0);
    ASSERT_EQ(g_cb.putglyph[1].col, 13);
    ASSERT_EQ(g_cb.putglyph[2].chars[0], 0x61);
    ASSERT_EQ(g_cb.putglyph[2].width, 1);
    ASSERT_EQ(g_cb.putglyph[2].row, 0);
    ASSERT_EQ(g_cb.putglyph[2].col, 14);
}

// LS1R
TEST(state_encoding_ls1r)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);

    state.reset(true);
    callbacks_clear();
    push(vt, "\e~");
    push(vt, {"\xe1", 1});
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 0x61);
    ASSERT_EQ(g_cb.putglyph[0].width, 1);
    ASSERT_EQ(g_cb.putglyph[0].row, 0);
    ASSERT_EQ(g_cb.putglyph[0].col, 0);

    callbacks_clear();
    push(vt, "\e)0");
    push(vt, {"\xe1", 1});
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 0x2592);
    ASSERT_EQ(g_cb.putglyph[0].width, 1);
    ASSERT_EQ(g_cb.putglyph[0].row, 0);
    ASSERT_EQ(g_cb.putglyph[0].col, 1);
}

// LS2R
TEST(state_encoding_ls2r)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);

    state.reset(true);
    callbacks_clear();
    push(vt, "\e}");
    push(vt, {"\xe1", 1});
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 0x61);
    ASSERT_EQ(g_cb.putglyph[0].width, 1);
    ASSERT_EQ(g_cb.putglyph[0].row, 0);
    ASSERT_EQ(g_cb.putglyph[0].col, 0);

    callbacks_clear();
    push(vt, "\e*0");
    push(vt, {"\xe1", 1});
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 0x2592);
    ASSERT_EQ(g_cb.putglyph[0].width, 1);
    ASSERT_EQ(g_cb.putglyph[0].row, 0);
    ASSERT_EQ(g_cb.putglyph[0].col, 1);
}

// LS3R
TEST(state_encoding_ls3r)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);

    state.reset(true);
    callbacks_clear();
    push(vt, "\e|");
    push(vt, {"\xe1", 1});
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 0x61);
    ASSERT_EQ(g_cb.putglyph[0].width, 1);
    ASSERT_EQ(g_cb.putglyph[0].row, 0);
    ASSERT_EQ(g_cb.putglyph[0].col, 0);

    callbacks_clear();
    push(vt, "\e+0");
    push(vt, {"\xe1", 1});
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 0x2592);
    ASSERT_EQ(g_cb.putglyph[0].width, 1);
    ASSERT_EQ(g_cb.putglyph[0].row, 0);
    ASSERT_EQ(g_cb.putglyph[0].col, 1);
}

// Mixed US-ASCII and UTF-8
TEST(state_encoding_mixed_usascii_utf8)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);

    // U+0108 == 0xc4 0x88
    state.reset(true);
    callbacks_clear();
    push(vt, "\e(B");
    push(vt, "AB\xc4\x88" "D");
    ASSERT_EQ(g_cb.putglyph_count, 4);
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 0x0041);
    ASSERT_EQ(g_cb.putglyph[0].width, 1);
    ASSERT_EQ(g_cb.putglyph[0].row, 0);
    ASSERT_EQ(g_cb.putglyph[0].col, 0);
    ASSERT_EQ(g_cb.putglyph[1].chars[0], 0x0042);
    ASSERT_EQ(g_cb.putglyph[1].width, 1);
    ASSERT_EQ(g_cb.putglyph[1].row, 0);
    ASSERT_EQ(g_cb.putglyph[1].col, 1);
    ASSERT_EQ(g_cb.putglyph[2].chars[0], 0x0108);
    ASSERT_EQ(g_cb.putglyph[2].width, 1);
    ASSERT_EQ(g_cb.putglyph[2].row, 0);
    ASSERT_EQ(g_cb.putglyph[2].col, 2);
    ASSERT_EQ(g_cb.putglyph[3].chars[0], 0x0044);
    ASSERT_EQ(g_cb.putglyph[3].width, 1);
    ASSERT_EQ(g_cb.putglyph[3].row, 0);
    ASSERT_EQ(g_cb.putglyph[3].col, 3);
}
