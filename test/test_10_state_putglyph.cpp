// test_10_state_putglyph.cpp — state putglyph tests
// Ported from upstream libvterm t/10state_putglyph.test

#include "harness.h"

// Low
TEST(state_putglyph_low)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);

    state.reset(true);
    callbacks_clear();

    push(vt, "ABC");
    ASSERT_EQ(g_cb.putglyph_count, 3);
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 0x41);
    ASSERT_EQ(g_cb.putglyph[0].width, 1);
    ASSERT_EQ(g_cb.putglyph[0].row, 0);
    ASSERT_EQ(g_cb.putglyph[0].col, 0);
    ASSERT_EQ(g_cb.putglyph[1].chars[0], 0x42);
    ASSERT_EQ(g_cb.putglyph[1].width, 1);
    ASSERT_EQ(g_cb.putglyph[1].row, 0);
    ASSERT_EQ(g_cb.putglyph[1].col, 1);
    ASSERT_EQ(g_cb.putglyph[2].chars[0], 0x43);
    ASSERT_EQ(g_cb.putglyph[2].width, 1);
    ASSERT_EQ(g_cb.putglyph[2].row, 0);
    ASSERT_EQ(g_cb.putglyph[2].col, 2);
}

// UTF-8 1 char
TEST(state_putglyph_utf8_1char)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);

    state.reset(true);
    callbacks_clear();

    // U+00C1 = 0xC3 0x81  LATIN CAPITAL LETTER A WITH ACUTE
    // U+00E9 = 0xC3 0xA9  LATIN SMALL LETTER E WITH ACUTE
    push(vt, "\xC3\x81\xC3\xA9");
    ASSERT_EQ(g_cb.putglyph_count, 2);
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 0xc1);
    ASSERT_EQ(g_cb.putglyph[0].width, 1);
    ASSERT_EQ(g_cb.putglyph[0].row, 0);
    ASSERT_EQ(g_cb.putglyph[0].col, 0);
    ASSERT_EQ(g_cb.putglyph[1].chars[0], 0xe9);
    ASSERT_EQ(g_cb.putglyph[1].width, 1);
    ASSERT_EQ(g_cb.putglyph[1].row, 0);
    ASSERT_EQ(g_cb.putglyph[1].col, 1);
}

// UTF-8 split writes
TEST(state_putglyph_utf8_split)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);

    state.reset(true);
    callbacks_clear();

    push(vt, "\xC3");
    ASSERT_EQ(g_cb.putglyph_count, 0);
    push(vt, "\x81");
    ASSERT_EQ(g_cb.putglyph_count, 1);
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 0xc1);
    ASSERT_EQ(g_cb.putglyph[0].width, 1);
    ASSERT_EQ(g_cb.putglyph[0].row, 0);
    ASSERT_EQ(g_cb.putglyph[0].col, 0);
}

// UTF-8 wide char
TEST(state_putglyph_utf8_wide)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);

    state.reset(true);
    callbacks_clear();

    // U+FF10 = 0xEF 0xBC 0x90  FULLWIDTH DIGIT ZERO
    push(vt, "\xEF\xBC\x90 ");
    ASSERT_EQ(g_cb.putglyph_count, 2);
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 0xff10);
    ASSERT_EQ(g_cb.putglyph[0].width, 2);
    ASSERT_EQ(g_cb.putglyph[0].row, 0);
    ASSERT_EQ(g_cb.putglyph[0].col, 0);
    ASSERT_EQ(g_cb.putglyph[1].chars[0], 0x20);
    ASSERT_EQ(g_cb.putglyph[1].width, 1);
    ASSERT_EQ(g_cb.putglyph[1].row, 0);
    ASSERT_EQ(g_cb.putglyph[1].col, 2);
}

// UTF-8 emoji wide char
TEST(state_putglyph_utf8_emoji_wide)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);

    state.reset(true);
    callbacks_clear();

    // U+1F600 = 0xF0 0x9F 0x98 0x80  GRINNING FACE
    push(vt, "\xF0\x9F\x98\x80 ");
    ASSERT_EQ(g_cb.putglyph_count, 2);
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 0x1f600);
    ASSERT_EQ(g_cb.putglyph[0].width, 2);
    ASSERT_EQ(g_cb.putglyph[0].row, 0);
    ASSERT_EQ(g_cb.putglyph[0].col, 0);
    ASSERT_EQ(g_cb.putglyph[1].chars[0], 0x20);
    ASSERT_EQ(g_cb.putglyph[1].width, 1);
    ASSERT_EQ(g_cb.putglyph[1].row, 0);
    ASSERT_EQ(g_cb.putglyph[1].col, 2);
}

// UTF-8 combining chars
TEST(state_putglyph_utf8_combining)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);

    state.reset(true);
    callbacks_clear();

    // U+0301 = 0xCC 0x81  COMBINING ACUTE
    push(vt, "e\xCC\x81Z");
    ASSERT_EQ(g_cb.putglyph_count, 2);
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 0x65);
    ASSERT_EQ(g_cb.putglyph[0].chars[1], 0x301);
    ASSERT_EQ(g_cb.putglyph[0].width, 1);
    ASSERT_EQ(g_cb.putglyph[0].row, 0);
    ASSERT_EQ(g_cb.putglyph[0].col, 0);
    ASSERT_EQ(g_cb.putglyph[1].chars[0], 0x5a);
    ASSERT_EQ(g_cb.putglyph[1].width, 1);
    ASSERT_EQ(g_cb.putglyph[1].row, 0);
    ASSERT_EQ(g_cb.putglyph[1].col, 1);
}

// Combining across buffers
TEST(state_putglyph_combining_across_buffers)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);

    state.reset(true);
    callbacks_clear();

    push(vt, "e");
    ASSERT_EQ(g_cb.putglyph_count, 1);
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 0x65);
    ASSERT_EQ(g_cb.putglyph[0].width, 1);
    ASSERT_EQ(g_cb.putglyph[0].row, 0);
    ASSERT_EQ(g_cb.putglyph[0].col, 0);

    callbacks_clear();

    push(vt, "\xCC\x81Z");
    ASSERT_EQ(g_cb.putglyph_count, 2);
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 0x65);
    ASSERT_EQ(g_cb.putglyph[0].chars[1], 0x301);
    ASSERT_EQ(g_cb.putglyph[0].width, 1);
    ASSERT_EQ(g_cb.putglyph[0].row, 0);
    ASSERT_EQ(g_cb.putglyph[0].col, 0);
    ASSERT_EQ(g_cb.putglyph[1].chars[0], 0x5a);
    ASSERT_EQ(g_cb.putglyph[1].width, 1);
    ASSERT_EQ(g_cb.putglyph[1].row, 0);
    ASSERT_EQ(g_cb.putglyph[1].col, 1);
}

// Spare combining chars get truncated
TEST(state_putglyph_combining_truncated)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);

    state.reset(true);
    callbacks_clear();

    // "e" . "\xCC\x81" x 10 — e followed by combining acute 10 times
    push(vt, "e"
         "\xCC\x81" "\xCC\x81" "\xCC\x81" "\xCC\x81" "\xCC\x81"
         "\xCC\x81" "\xCC\x81" "\xCC\x81" "\xCC\x81" "\xCC\x81");
    // Only 6 chars stored: base + 5 combiners; rest truncated
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 0x65);
    ASSERT_EQ(g_cb.putglyph[0].chars[1], 0x301);
    ASSERT_EQ(g_cb.putglyph[0].chars[2], 0x301);
    ASSERT_EQ(g_cb.putglyph[0].chars[3], 0x301);
    ASSERT_EQ(g_cb.putglyph[0].chars[4], 0x301);
    ASSERT_EQ(g_cb.putglyph[0].chars[5], 0x301);
    ASSERT_EQ(g_cb.putglyph[0].width, 1);
    ASSERT_EQ(g_cb.putglyph[0].row, 0);
    ASSERT_EQ(g_cb.putglyph[0].col, 0);

    // Second part: incremental combining across pushes
    state.reset(true);
    callbacks_clear();

    push(vt, "e");
    ASSERT_EQ(g_cb.putglyph_count, 1);
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 0x65);
    ASSERT_EQ(g_cb.putglyph[0].width, 1);
    ASSERT_EQ(g_cb.putglyph[0].row, 0);
    ASSERT_EQ(g_cb.putglyph[0].col, 0);

    callbacks_clear();

    push(vt, "\xCC\x81");
    ASSERT_EQ(g_cb.putglyph_count, 1);
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 0x65);
    ASSERT_EQ(g_cb.putglyph[0].chars[1], 0x301);
    ASSERT_EQ(g_cb.putglyph[0].width, 1);
    ASSERT_EQ(g_cb.putglyph[0].row, 0);
    ASSERT_EQ(g_cb.putglyph[0].col, 0);

    callbacks_clear();

    push(vt, "\xCC\x82");
    ASSERT_EQ(g_cb.putglyph_count, 1);
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 0x65);
    ASSERT_EQ(g_cb.putglyph[0].chars[1], 0x301);
    ASSERT_EQ(g_cb.putglyph[0].chars[2], 0x302);
    ASSERT_EQ(g_cb.putglyph[0].width, 1);
    ASSERT_EQ(g_cb.putglyph[0].row, 0);
    ASSERT_EQ(g_cb.putglyph[0].col, 0);
}

// DECSCA protected
TEST(state_putglyph_decsca_protected)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);

    state.reset(true);
    callbacks_clear();

    // A, then DECSCA enable (1"q), B, then DECSCA disable (2"q), C
    push(vt, "A\e[1\"qB\e[2\"qC");
    ASSERT_EQ(g_cb.putglyph_count, 3);
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 0x41);
    ASSERT_EQ(g_cb.putglyph[0].width, 1);
    ASSERT_EQ(g_cb.putglyph[0].row, 0);
    ASSERT_EQ(g_cb.putglyph[0].col, 0);
    ASSERT_EQ(g_cb.putglyph[0].protected_cell, false);
    ASSERT_EQ(g_cb.putglyph[1].chars[0], 0x42);
    ASSERT_EQ(g_cb.putglyph[1].width, 1);
    ASSERT_EQ(g_cb.putglyph[1].row, 0);
    ASSERT_EQ(g_cb.putglyph[1].col, 1);
    ASSERT_EQ(g_cb.putglyph[1].protected_cell, true);
    ASSERT_EQ(g_cb.putglyph[2].chars[0], 0x43);
    ASSERT_EQ(g_cb.putglyph[2].width, 1);
    ASSERT_EQ(g_cb.putglyph[2].row, 0);
    ASSERT_EQ(g_cb.putglyph[2].col, 2);
    ASSERT_EQ(g_cb.putglyph[2].protected_cell, false);
}
