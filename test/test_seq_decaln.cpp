// test_seq_decaln.cpp -- tests for DECALN (ESC # 8) Screen Alignment Display
//
// DECALN fills the entire screen with 'E' characters. This is used
// for screen alignment testing.

#include "harness.h"

// ===== DECALN fills screen with E via screen layer =====
TEST(seq_decaln_fills_with_E)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    Screen& screen = vt.screen();
    screen.set_callbacks(screen_cbs);
    screen.reset(true);

    // Send DECALN: ESC # 8
    push(vt, "\e#8");

    // Build expected string of 80 E's
    std::array<char, 81> expected{};
    expected.fill('E');
    expected[80] = '\0';

    // Verify first and last rows are all E's
    ASSERT_SCREEN_ROW(vt, screen, 0, expected.data());
    ASSERT_SCREEN_ROW(vt, screen, 24, expected.data());
}

// ===== DECALN generates putglyph callbacks =====
TEST(seq_decaln_putglyph_count)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Send DECALN: ESC # 8
    push(vt, "\e#8");

        // DECALN should produce 25*80 = 2000 putglyph callbacks.
    // The callback log can only store CALLBACK_LOG_MAX (128),
    // but putglyph_count tracks the total.
    ASSERT_EQ(g_cb.putglyph_count, 2000);

    // Verify the first few recorded glyphs are 'E'
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 'E');
    ASSERT_EQ(g_cb.putglyph[0].row, 0);
    ASSERT_EQ(g_cb.putglyph[0].col, 0);
    ASSERT_EQ(g_cb.putglyph[1].chars[0], 'E');
    ASSERT_EQ(g_cb.putglyph[1].row, 0);
    ASSERT_EQ(g_cb.putglyph[1].col, 1);
}

// ===== DECALN screen content across multiple rows =====
TEST(seq_decaln_screen_content)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    Screen& screen = vt.screen();
    screen.set_callbacks(screen_cbs);
    screen.reset(true);

    // First write some other content
    push(vt, "Hello, world!");

    // Now send DECALN -- should overwrite everything
    push(vt, "\e#8");

    // Build expected string of 80 E's
    std::array<char, 81> expected{};
    expected.fill('E');
    expected[80] = '\0';

    // Verify several rows
    ASSERT_SCREEN_ROW(vt, screen, 0, expected.data());
    ASSERT_SCREEN_ROW(vt, screen, 1, expected.data());
    ASSERT_SCREEN_ROW(vt, screen, 12, expected.data());
    ASSERT_SCREEN_ROW(vt, screen, 24, expected.data());
}
