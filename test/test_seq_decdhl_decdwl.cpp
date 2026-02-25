// test_seq_decdhl_decdwl.cpp -- tests for DECDHL/DECDWL/DECSWL
//
// DECSWL (ESC # 5) - Single-Width, Single-Height line
// DECDWL (ESC # 6) - Double-Width, Single-Height line
// DECDHL top    (ESC # 3) - Double-Width, Double-Height line (top half)
// DECDHL bottom (ESC # 4) - Double-Width, Double-Height line (bottom half)
//
// The dwl and dhl fields in putglyph_record indicate:
//   dwl=0 -> single width
//   dwl=1 -> double width
//   dhl=0 -> single height
//   dhl=1 -> double height top half
//   dhl=2 -> double height bottom half

#include "harness.h"

// ===== DECDWL: double-width line =====
TEST(seq_decdwl_double_width)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Set DECDWL: ESC # 6
    push(vt, "\e#6");
    push(vt, "A");

    ASSERT_EQ(g_cb.putglyph_count, 1);
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 'A');
    ASSERT_EQ(g_cb.putglyph[0].width, 1);
    ASSERT_EQ(g_cb.putglyph[0].dwl, 1);
    ASSERT_EQ(g_cb.putglyph[0].dhl, 0);
    ASSERT_EQ(g_cb.putglyph[0].row, 0);
    ASSERT_EQ(g_cb.putglyph[0].col, 0);
}

// ===== DECSWL: single-width line =====
TEST(seq_decswl_single_width)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Set DECSWL: ESC # 5
    push(vt, "\e#5");
    push(vt, "A");

    ASSERT_EQ(g_cb.putglyph_count, 1);
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 'A');
    ASSERT_EQ(g_cb.putglyph[0].width, 1);
    ASSERT_EQ(g_cb.putglyph[0].dwl, 0);
    ASSERT_EQ(g_cb.putglyph[0].dhl, 0);
    ASSERT_EQ(g_cb.putglyph[0].row, 0);
    ASSERT_EQ(g_cb.putglyph[0].col, 0);
}

// ===== DECDHL top half =====
TEST(seq_decdhl_top_half)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Set DECDHL top half: ESC # 3
    push(vt, "\e#3");
    push(vt, "A");

    ASSERT_EQ(g_cb.putglyph_count, 1);
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 'A');
    ASSERT_EQ(g_cb.putglyph[0].width, 1);
    ASSERT_EQ(g_cb.putglyph[0].dwl, 1);
    ASSERT_EQ(g_cb.putglyph[0].dhl, 1);
    ASSERT_EQ(g_cb.putglyph[0].row, 0);
    ASSERT_EQ(g_cb.putglyph[0].col, 0);
}

// ===== DECDHL bottom half =====
TEST(seq_decdhl_bottom_half)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Set DECDHL bottom half: ESC # 4
    push(vt, "\e#4");
    push(vt, "A");

    ASSERT_EQ(g_cb.putglyph_count, 1);
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 'A');
    ASSERT_EQ(g_cb.putglyph[0].width, 1);
    ASSERT_EQ(g_cb.putglyph[0].dwl, 1);
    ASSERT_EQ(g_cb.putglyph[0].dhl, 2);
    ASSERT_EQ(g_cb.putglyph[0].row, 0);
    ASSERT_EQ(g_cb.putglyph[0].col, 0);
}
