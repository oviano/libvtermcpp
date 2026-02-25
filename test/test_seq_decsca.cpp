// test_seq_decsca.cpp -- tests for DECSCA (CSI " q) Select Character Protection Attribute
//
// DECSCA controls whether characters printed after it are "protected"
// against selective erase operations.
//   CSI 1 " q  -> enable protection
//   CSI 0 " q  -> disable protection
//   CSI 2 " q  -> disable protection
//   CSI " q    -> default (0 = disable)

#include "harness.h"

// ===== DECSCA 1 enables protection =====
TEST(seq_decsca_protect)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Enable protection, then print 'A'
    push(vt, "\e[1\"q");
    push(vt, "A");

    ASSERT_EQ(g_cb.putglyph_count, 1);
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 'A');
    ASSERT_EQ(g_cb.putglyph[0].protected_cell, true);
}

// ===== DECSCA 0 disables protection =====
TEST(seq_decsca_unprotect_0)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // First enable protection, then disable with param 0
    push(vt, "\e[1\"q");
    push(vt, "\e[0\"q");
    push(vt, "B");

    ASSERT_EQ(g_cb.putglyph_count, 1);
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 'B');
    ASSERT_EQ(g_cb.putglyph[0].protected_cell, false);
}

// ===== DECSCA 2 also disables protection =====
TEST(seq_decsca_unprotect_2)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // First enable protection, then disable with param 2
    push(vt, "\e[1\"q");
    push(vt, "\e[2\"q");
    push(vt, "C");

    ASSERT_EQ(g_cb.putglyph_count, 1);
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 'C');
    ASSERT_EQ(g_cb.putglyph[0].protected_cell, false);
}

// ===== DECSCA with no param defaults to 0 (unprotect) =====
TEST(seq_decsca_default)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Enable protection first
    push(vt, "\e[1\"q");

    // Default DECSCA (no param) should disable protection
    push(vt, "\e[\"q");
    push(vt, "D");

    ASSERT_EQ(g_cb.putglyph_count, 1);
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 'D');
    ASSERT_EQ(g_cb.putglyph[0].protected_cell, false);
}

// ===== DECSCA flag in putglyph toggles correctly =====
TEST(seq_decsca_putglyph_flag)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Enable protection, print 'X'
    push(vt, "\e[1\"q");
    push(vt, "X");

    ASSERT_EQ(g_cb.putglyph_count, 1);
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 'X');
    ASSERT_EQ(g_cb.putglyph[0].protected_cell, true);

    // Disable protection, print 'Y'
    push(vt, "\e[2\"q");
    push(vt, "Y");

    ASSERT_EQ(g_cb.putglyph_count, 2);
    ASSERT_EQ(g_cb.putglyph[1].chars[0], 'Y');
    ASSERT_EQ(g_cb.putglyph[1].protected_cell, false);
}
