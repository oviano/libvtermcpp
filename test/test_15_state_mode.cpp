// test_15_state_mode.cpp — state mode tests
// Ported from upstream libvterm t/15state_mode.test

#include "harness.h"

// Insert/Replace Mode
TEST(state_mode_insert_replace)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs_no_scrollrect);
    output_init(vt);

    // RESET
    state.reset(true);

    // erase 0..25,0..80
    ASSERT_EQ(g_cb.erase_count, 1);
    ASSERT_EQ(g_cb.erase[0].rect.start_row, 0);
    ASSERT_EQ(g_cb.erase[0].rect.end_row, 25);
    ASSERT_EQ(g_cb.erase[0].rect.start_col, 0);
    ASSERT_EQ(g_cb.erase[0].rect.end_col, 80);

    // ?cursor = 0,0
    ASSERT_CURSOR(state, 0, 0);

    callbacks_clear();
    push(vt, "AC\x1b[DB");

    // putglyph 0x41 1 0,0
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 0x41);
    ASSERT_EQ(g_cb.putglyph[0].width, 1);
    ASSERT_EQ(g_cb.putglyph[0].row, 0);
    ASSERT_EQ(g_cb.putglyph[0].col, 0);

    // putglyph 0x43 1 0,1
    ASSERT_EQ(g_cb.putglyph[1].chars[0], 0x43);
    ASSERT_EQ(g_cb.putglyph[1].width, 1);
    ASSERT_EQ(g_cb.putglyph[1].row, 0);
    ASSERT_EQ(g_cb.putglyph[1].col, 1);

    // putglyph 0x42 1 0,1
    ASSERT_EQ(g_cb.putglyph[2].chars[0], 0x42);
    ASSERT_EQ(g_cb.putglyph[2].width, 1);
    ASSERT_EQ(g_cb.putglyph[2].row, 0);
    ASSERT_EQ(g_cb.putglyph[2].col, 1);

    // Set insert mode: CSI 4h
    push(vt, "\x1b[4h");

    // Move cursor to column 0: CSI G
    push(vt, "\x1b[G");

    callbacks_clear();
    push(vt, "AC\x1b[DB");

    // moverect 0..1,0..79 -> 0..1,1..80
    ASSERT_EQ(g_cb.moverect[0].src.start_row, 0);
    ASSERT_EQ(g_cb.moverect[0].src.end_row, 1);
    ASSERT_EQ(g_cb.moverect[0].src.start_col, 0);
    ASSERT_EQ(g_cb.moverect[0].src.end_col, 79);
    ASSERT_EQ(g_cb.moverect[0].dest.start_row, 0);
    ASSERT_EQ(g_cb.moverect[0].dest.end_row, 1);
    ASSERT_EQ(g_cb.moverect[0].dest.start_col, 1);
    ASSERT_EQ(g_cb.moverect[0].dest.end_col, 80);

    // erase 0..1,0..1
    ASSERT_EQ(g_cb.erase[0].rect.start_row, 0);
    ASSERT_EQ(g_cb.erase[0].rect.end_row, 1);
    ASSERT_EQ(g_cb.erase[0].rect.start_col, 0);
    ASSERT_EQ(g_cb.erase[0].rect.end_col, 1);

    // putglyph 0x41 1 0,0
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 0x41);
    ASSERT_EQ(g_cb.putglyph[0].width, 1);
    ASSERT_EQ(g_cb.putglyph[0].row, 0);
    ASSERT_EQ(g_cb.putglyph[0].col, 0);

    // moverect 0..1,1..79 -> 0..1,2..80
    ASSERT_EQ(g_cb.moverect[1].src.start_row, 0);
    ASSERT_EQ(g_cb.moverect[1].src.end_row, 1);
    ASSERT_EQ(g_cb.moverect[1].src.start_col, 1);
    ASSERT_EQ(g_cb.moverect[1].src.end_col, 79);
    ASSERT_EQ(g_cb.moverect[1].dest.start_row, 0);
    ASSERT_EQ(g_cb.moverect[1].dest.end_row, 1);
    ASSERT_EQ(g_cb.moverect[1].dest.start_col, 2);
    ASSERT_EQ(g_cb.moverect[1].dest.end_col, 80);

    // erase 0..1,1..2
    ASSERT_EQ(g_cb.erase[1].rect.start_row, 0);
    ASSERT_EQ(g_cb.erase[1].rect.end_row, 1);
    ASSERT_EQ(g_cb.erase[1].rect.start_col, 1);
    ASSERT_EQ(g_cb.erase[1].rect.end_col, 2);

    // putglyph 0x43 1 0,1
    ASSERT_EQ(g_cb.putglyph[1].chars[0], 0x43);
    ASSERT_EQ(g_cb.putglyph[1].width, 1);
    ASSERT_EQ(g_cb.putglyph[1].row, 0);
    ASSERT_EQ(g_cb.putglyph[1].col, 1);

    // moverect 0..1,1..79 -> 0..1,2..80
    ASSERT_EQ(g_cb.moverect[2].src.start_row, 0);
    ASSERT_EQ(g_cb.moverect[2].src.end_row, 1);
    ASSERT_EQ(g_cb.moverect[2].src.start_col, 1);
    ASSERT_EQ(g_cb.moverect[2].src.end_col, 79);
    ASSERT_EQ(g_cb.moverect[2].dest.start_row, 0);
    ASSERT_EQ(g_cb.moverect[2].dest.end_row, 1);
    ASSERT_EQ(g_cb.moverect[2].dest.start_col, 2);
    ASSERT_EQ(g_cb.moverect[2].dest.end_col, 80);

    // erase 0..1,1..2
    ASSERT_EQ(g_cb.erase[2].rect.start_row, 0);
    ASSERT_EQ(g_cb.erase[2].rect.end_row, 1);
    ASSERT_EQ(g_cb.erase[2].rect.start_col, 1);
    ASSERT_EQ(g_cb.erase[2].rect.end_col, 2);

    // putglyph 0x42 1 0,1
    ASSERT_EQ(g_cb.putglyph[2].chars[0], 0x42);
    ASSERT_EQ(g_cb.putglyph[2].width, 1);
    ASSERT_EQ(g_cb.putglyph[2].row, 0);
    ASSERT_EQ(g_cb.putglyph[2].col, 1);
}

// Insert mode only happens once for UTF-8 combining
TEST(state_mode_insert_utf8_combining)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs_no_scrollrect);
    output_init(vt);

        // Reproduce state from previous section:
    // RESET, then "AC\e[DB" in replace mode, then set insert mode,
    // go to col 0, then "AC\e[DB" in insert mode.
    // Cursor ends at 0,2. Insert mode is still on.
    state.reset(true);
    push(vt, "AC\x1b[DB");
    push(vt, "\x1b[4h");
    push(vt, "\x1b[G");
    push(vt, "AC\x1b[DB");

    callbacks_clear();
    push(vt, "e");

    // moverect 0..1,2..79 -> 0..1,3..80
    ASSERT_EQ(g_cb.moverect[0].src.start_row, 0);
    ASSERT_EQ(g_cb.moverect[0].src.end_row, 1);
    ASSERT_EQ(g_cb.moverect[0].src.start_col, 2);
    ASSERT_EQ(g_cb.moverect[0].src.end_col, 79);
    ASSERT_EQ(g_cb.moverect[0].dest.start_row, 0);
    ASSERT_EQ(g_cb.moverect[0].dest.end_row, 1);
    ASSERT_EQ(g_cb.moverect[0].dest.start_col, 3);
    ASSERT_EQ(g_cb.moverect[0].dest.end_col, 80);

    // erase 0..1,2..3
    ASSERT_EQ(g_cb.erase[0].rect.start_row, 0);
    ASSERT_EQ(g_cb.erase[0].rect.end_row, 1);
    ASSERT_EQ(g_cb.erase[0].rect.start_col, 2);
    ASSERT_EQ(g_cb.erase[0].rect.end_col, 3);

    // putglyph 0x65 1 0,2
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 0x65);
    ASSERT_EQ(g_cb.putglyph[0].width, 1);
    ASSERT_EQ(g_cb.putglyph[0].row, 0);
    ASSERT_EQ(g_cb.putglyph[0].col, 2);

    callbacks_clear();
    push(vt, "\xCC\x81");

    // putglyph 0x65,0x301 1 0,2 -- combining character, no extra moverect
    ASSERT_EQ(g_cb.putglyph[0].chars[0], 0x65);
    ASSERT_EQ(g_cb.putglyph[0].chars[1], 0x301);
    ASSERT_EQ(g_cb.putglyph[0].width, 1);
    ASSERT_EQ(g_cb.putglyph[0].row, 0);
    ASSERT_EQ(g_cb.putglyph[0].col, 2);

    // No moverect for combining -- insert mode only fires once
    ASSERT_EQ(g_cb.moverect_count, 0);
}

// Newline/Linefeed mode
TEST(state_mode_newline_linefeed)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs_no_scrollrect);
    output_init(vt);

    // RESET
    state.reset(true);

    // erase 0..25,0..80
    ASSERT_EQ(g_cb.erase_count, 1);
    ASSERT_EQ(g_cb.erase[0].rect.start_row, 0);
    ASSERT_EQ(g_cb.erase[0].rect.end_row, 25);
    ASSERT_EQ(g_cb.erase[0].rect.start_col, 0);
    ASSERT_EQ(g_cb.erase[0].rect.end_col, 80);

    // ?cursor = 0,0
    ASSERT_CURSOR(state, 0, 0);

    callbacks_clear();

    // Move to column 5, then newline
    push(vt, "\x1b[5G\n");
    // ?cursor = 1,4 -- column stays at 4 (0-based for column 5)
    ASSERT_CURSOR(state, 1, 4);

    // Enable newline mode: CSI 20h
    push(vt, "\x1b[20h");

    // Move to column 5, then newline
    push(vt, "\x1b[5G\n");
    // ?cursor = 2,0 -- newline mode means LF also does CR
    ASSERT_CURSOR(state, 2, 0);
}

// DEC origin mode
TEST(state_mode_dec_origin)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs_no_scrollrect);
    output_init(vt);

    // RESET
    state.reset(true);

    // erase 0..25,0..80
    ASSERT_EQ(g_cb.erase_count, 1);

    // ?cursor = 0,0
    ASSERT_CURSOR(state, 0, 0);

    callbacks_clear();

    // Set scroll region rows 5..15: CSI 5;15 r
    push(vt, "\x1b[5;15r");

    // Home: CSI H
    push(vt, "\x1b[H");
    // ?cursor = 0,0 -- no origin mode yet
    ASSERT_CURSOR(state, 0, 0);

    // Move to row 3, col 3
    push(vt, "\x1b[3;3H");
    // ?cursor = 2,2
    ASSERT_CURSOR(state, 2, 2);

    // Enable DEC origin mode: CSI ?6h
    push(vt, "\x1b[?6h");

    // Home: CSI H
    push(vt, "\x1b[H");
    // ?cursor = 4,0 -- origin mode puts cursor at top of scroll region
    ASSERT_CURSOR(state, 4, 0);

    // Move to row 3, col 3 within scroll region
    push(vt, "\x1b[3;3H");
    // ?cursor = 6,2 -- row 3 within scroll region (rows 5..15) = row 6
    ASSERT_CURSOR(state, 6, 2);
}

// DECRQM on DECOM
TEST(state_mode_decrqm_decom)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs_no_scrollrect);
    output_init(vt);

        // Reproduce state from DEC origin mode section:
    // RESET, set scroll region 5..15, already has it.
    state.reset(true);
    push(vt, "\x1b[5;15r");

    // Enable DECOM
    push(vt, "\x1b[?6h");

    // Query DECOM: CSI ?6$p
    output_clear();
    push(vt, "\x1b[?6$p");
    // output "\e[?6;1$y" -- mode 6 set (1)
    ASSERT_OUTPUT_BYTES("\x1b[?6;1$y", 8);

    // Disable DECOM
    push(vt, "\x1b[?6l");

    // Query DECOM: CSI ?6$p
    output_clear();
    push(vt, "\x1b[?6$p");
    // output "\e[?6;2$y" -- mode 6 reset (2)
    ASSERT_OUTPUT_BYTES("\x1b[?6;2$y", 8);
}

// Origin mode with DECSLRM
TEST(state_mode_origin_decslrm)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs_no_scrollrect);
    output_init(vt);

        // Reproduce state: RESET, scroll region 5..15, DECOM was toggled.
    // We need scroll region and origin mode active.
    state.reset(true);
    push(vt, "\x1b[5;15r");

    // Enable origin mode
    push(vt, "\x1b[?6h");

    // Enable left/right margin mode: CSI ?69h
    push(vt, "\x1b[?69h");

    // Set left/right margins: CSI 20;60 s
    push(vt, "\x1b[20;60s");

    // Home
    push(vt, "\x1b[H");
    // ?cursor = 4,19 -- origin mode, row 5 (idx 4), col 20 (idx 19)
    ASSERT_CURSOR(state, 4, 19);

    // Disable left/right margin mode
    push(vt, "\x1b[?69l");
}

// Origin mode bounds cursor to scrolling region
TEST(state_mode_origin_bounds_cursor)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs_no_scrollrect);
    output_init(vt);

        // Reproduce state: RESET, scroll region 5..15, origin mode on,
    // DECSLRM was enabled then disabled.
    state.reset(true);
    push(vt, "\x1b[5;15r");
    push(vt, "\x1b[?6h");
    push(vt, "\x1b[?69h");
    push(vt, "\x1b[20;60s");
    push(vt, "\x1b[?69l");

    // Home
    push(vt, "\x1b[H");

    // Try to move 10 rows up -- should be clamped to top of scroll region
    push(vt, "\x1b[10A");
    // ?cursor = 4,0
    ASSERT_CURSOR(state, 4, 0);

    // Try to move 20 rows down -- should be clamped to bottom of scroll region
    push(vt, "\x1b[20B");
    // ?cursor = 14,0
    ASSERT_CURSOR(state, 14, 0);
}

// Origin mode without scroll region
TEST(state_mode_origin_no_scroll_region)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs_no_scrollrect);
    output_init(vt);

    // Reproduce state: RESET, scroll region 5..15, origin mode on
    state.reset(true);
    push(vt, "\x1b[5;15r");
    push(vt, "\x1b[?6h");

    // Disable origin mode
    push(vt, "\x1b[?6l");

    // Reset scroll region and re-enable origin mode
    push(vt, "\x1b[r\x1b[?6h");
    // ?cursor = 0,0
    ASSERT_CURSOR(state, 0, 0);
}
