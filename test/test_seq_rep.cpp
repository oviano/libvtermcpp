// test_seq_rep.cpp -- per-sequence tests for REP (CSI b)
// Repeat Preceding Graphic Character

#include "harness.h"

// REP with no param defaults to 1
TEST(seq_rep_default_param)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    push(vt, "A");
    push(vt, "\e[b");

    // Original A + 1 repeat = 2 putglyphs
    ASSERT_EQ(g_cb.putglyph_count, 2);
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 0x41);
    ASSERT_EQ(g_cb.putglyph[1].chars[0], 0x41);
}

// REP with explicit count
TEST(seq_rep_explicit_count)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    push(vt, "A");
    push(vt, "\e[5b");

    // Original A + 5 repeats = 6 putglyphs
    ASSERT_EQ(g_cb.putglyph_count, 6);
    for (int32_t i = 0; i < 6; i++)
        ASSERT_EQ(g_cb.putglyph[i].chars[0], 0x41);
}

// REP screen content verification
TEST(seq_rep_screen_verify)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    Screen& screen = vt.screen();
    screen.set_callbacks(screen_cbs);
    screen.reset(true);

    push(vt, "X");
    push(vt, "\e[4b");

    ASSERT_SCREEN_ROW(vt, screen, 0, "XXXXX");
}

// REP with no preceding char does nothing
TEST(seq_rep_no_preceding_char)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    push(vt, "\e[5b");

    ASSERT_EQ(g_cb.putglyph_count, 0);
}

// REP near end of line with autowrap
TEST(seq_rep_with_autowrap)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Position at col 78 (0-based), print A at col 78
    push(vt, "\e[1;79H");
    push(vt, "A");
    // Cursor at col 79, in phantom state. REP overwrites at col 79
    push(vt, "\e[5b");
    // Cursor remains at col 79 in phantom state
    ASSERT_CURSOR(state, 0, 79);
}
