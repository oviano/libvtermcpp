// test_31_state_rep.cpp — state REP (repeat) tests
// Ported from upstream libvterm t/31state_rep.test

#include "harness.h"

// REP no argument
TEST(state_rep_no_argument)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);

    state.reset(true);
    callbacks_clear();

    push(vt, "a\x1b[b");
    ASSERT_EQ(g_cb.putglyph_count, 2);
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 0x61);
    ASSERT_EQ(g_cb.putglyph[0].width, 1);
    ASSERT_EQ(g_cb.putglyph[0].row, 0);
    ASSERT_EQ(g_cb.putglyph[0].col, 0);
    ASSERT_EQ(g_cb.putglyph[1].chars[0], 0x61);
    ASSERT_EQ(g_cb.putglyph[1].width, 1);
    ASSERT_EQ(g_cb.putglyph[1].row, 0);
    ASSERT_EQ(g_cb.putglyph[1].col, 1);
}

// REP zero (zero should be interpreted as one)
TEST(state_rep_zero)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);

    state.reset(true);
    callbacks_clear();

    push(vt, "a\x1b[0b");
    ASSERT_EQ(g_cb.putglyph_count, 2);
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 0x61);
    ASSERT_EQ(g_cb.putglyph[0].width, 1);
    ASSERT_EQ(g_cb.putglyph[0].row, 0);
    ASSERT_EQ(g_cb.putglyph[0].col, 0);
    ASSERT_EQ(g_cb.putglyph[1].chars[0], 0x61);
    ASSERT_EQ(g_cb.putglyph[1].width, 1);
    ASSERT_EQ(g_cb.putglyph[1].row, 0);
    ASSERT_EQ(g_cb.putglyph[1].col, 1);
}

// REP lowercase a times two
TEST(state_rep_times_two)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);

    state.reset(true);
    callbacks_clear();

    push(vt, "a\x1b[2b");
    ASSERT_EQ(g_cb.putglyph_count, 3);
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 0x61);
    ASSERT_EQ(g_cb.putglyph[0].width, 1);
    ASSERT_EQ(g_cb.putglyph[0].row, 0);
    ASSERT_EQ(g_cb.putglyph[0].col, 0);
    ASSERT_EQ(g_cb.putglyph[1].chars[0], 0x61);
    ASSERT_EQ(g_cb.putglyph[1].width, 1);
    ASSERT_EQ(g_cb.putglyph[1].row, 0);
    ASSERT_EQ(g_cb.putglyph[1].col, 1);
    ASSERT_EQ(g_cb.putglyph[2].chars[0], 0x61);
    ASSERT_EQ(g_cb.putglyph[2].width, 1);
    ASSERT_EQ(g_cb.putglyph[2].row, 0);
    ASSERT_EQ(g_cb.putglyph[2].col, 2);
}

// REP with UTF-8 1 char
TEST(state_rep_utf8_1char)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);

    // U+00E9 = 0xC3 0xA9  LATIN SMALL LETTER E WITH ACUTE
    state.reset(true);
    callbacks_clear();

    push(vt, "\xC3\xA9\x1b[b");
    ASSERT_EQ(g_cb.putglyph_count, 2);
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 0xe9);
    ASSERT_EQ(g_cb.putglyph[0].width, 1);
    ASSERT_EQ(g_cb.putglyph[0].row, 0);
    ASSERT_EQ(g_cb.putglyph[0].col, 0);
    ASSERT_EQ(g_cb.putglyph[1].chars[0], 0xe9);
    ASSERT_EQ(g_cb.putglyph[1].width, 1);
    ASSERT_EQ(g_cb.putglyph[1].row, 0);
    ASSERT_EQ(g_cb.putglyph[1].col, 1);
}

// REP with UTF-8 wide char
TEST(state_rep_utf8_wide)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);

    // U+FF10 = 0xEF 0xBC 0x90  FULLWIDTH DIGIT ZERO
    state.reset(true);
    callbacks_clear();

    push(vt, "\xEF\xBC\x90\x1b[b");
    ASSERT_EQ(g_cb.putglyph_count, 2);
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 0xff10);
    ASSERT_EQ(g_cb.putglyph[0].width, 2);
    ASSERT_EQ(g_cb.putglyph[0].row, 0);
    ASSERT_EQ(g_cb.putglyph[0].col, 0);
    ASSERT_EQ(g_cb.putglyph[1].chars[0], 0xff10);
    ASSERT_EQ(g_cb.putglyph[1].width, 2);
    ASSERT_EQ(g_cb.putglyph[1].row, 0);
    ASSERT_EQ(g_cb.putglyph[1].col, 2);
}

// REP with UTF-8 combining character
TEST(state_rep_utf8_combining)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);

    state.reset(true);
    callbacks_clear();

    // U+0301 = 0xCC 0x81  COMBINING ACUTE ACCENT
    push(vt, "e\xCC\x81\x1b[b");
    ASSERT_EQ(g_cb.putglyph_count, 2);
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 0x65);
    ASSERT_EQ(g_cb.putglyph[0].chars[1], 0x301);
    ASSERT_EQ(g_cb.putglyph[0].width, 1);
    ASSERT_EQ(g_cb.putglyph[0].row, 0);
    ASSERT_EQ(g_cb.putglyph[0].col, 0);
    ASSERT_EQ(g_cb.putglyph[1].chars[0], 0x65);
    ASSERT_EQ(g_cb.putglyph[1].chars[1], 0x301);
    ASSERT_EQ(g_cb.putglyph[1].width, 1);
    ASSERT_EQ(g_cb.putglyph[1].row, 0);
    ASSERT_EQ(g_cb.putglyph[1].col, 1);
}

// REP till end of line
TEST(state_rep_till_end_of_line)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);

    state.reset(true);
    callbacks_clear();

    // "a" then REP 1000 then "b"
    push(vt, "a\x1b[1000bb");

    // 1 initial 'a' + 79 repeated 'a' fills the 80-column line, then 'b' wraps to next line
    ASSERT_EQ(g_cb.putglyph_count, 81);

    // First putglyph: 'a' at 0,0
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 0x61);
    ASSERT_EQ(g_cb.putglyph[0].width, 1);
    ASSERT_EQ(g_cb.putglyph[0].row, 0);
    ASSERT_EQ(g_cb.putglyph[0].col, 0);

    // Repeated 'a' fills columns 1 through 79
    int32_t i;
    for (i = 1; i <= 79; i++) {
        ASSERT_EQ(g_cb.putglyph[i].chars[0], 0x61);
        ASSERT_EQ(g_cb.putglyph[i].width, 1);
        ASSERT_EQ(g_cb.putglyph[i].row, 0);
        ASSERT_EQ(g_cb.putglyph[i].col, i);
    }

    // Final 'b' wraps to row 1, col 0
    ASSERT_EQ(g_cb.putglyph[80].chars[0], 0x62);
    ASSERT_EQ(g_cb.putglyph[80].width, 1);
    ASSERT_EQ(g_cb.putglyph[80].row, 1);
    ASSERT_EQ(g_cb.putglyph[80].col, 0);
}
