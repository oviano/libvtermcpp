// test_seq_scrollback_reflow.cpp -- scrollback round-trip reflow tests
//
// Tests the code path where content is pushed into scrollback (via scrolling),
// then popped back and reflowed during a resize-wider operation. This exercises
// the sb_popline reflow path in resize_buffer().
//
// Key mechanism: for scrollback to be popped during resize, the on-screen
// reflow must free rows by joining wrapped lines. Those free rows are then
// backfilled from scrollback.

#include "harness.h"

// Helper: standard setup for scrollback reflow tests.
#define REFLOW_SETUP(rows, cols) \
    Terminal vt(25, 80); \
    vt.set_utf8(false); \
    State& state = vt.state(); \
    state.set_callbacks(state_cbs_no_scrollrect); \
    state.reset(true); \
    Screen& screen = vt.screen(); \
    screen.set_callbacks(screen_cbs_scrollback_reflow); \
    screen.enable_reflow(true); \
    vt.set_size((rows), (cols)); \
    screen.reset(true); \
    scrollback_clear(); \
    callbacks_clear()

// Basic scrollback round-trip: a short line scrolls into scrollback while
// wrapped content remains on screen. Resize wider joins the wrapped content,
// freeing a row that gets filled from scrollback.
//
// Initial state (4 rows, 10 cols):
//   row 0: AAAAABBBBB      (first 10 chars of wrapped line)
//   row 1: CCC        cont (continuation of wrapped line)
//   row 2: DD
//   row 3: EE              (cursor here)
//   scrollback: [SHORT1]
//
// After resize to 4 rows, 15 cols:
//   row 0: SHORT1          (popped from scrollback)
//   row 1: AAAAABBBBBCCC   (joined)
//   row 2: DD
//   row 3: EE
TEST(scrollback_reflow_basic_roundtrip)
{
    REFLOW_SETUP(4, 10);

    // SHORT1 on row 0
    push(vt, "SHORT1\r\n");
    // 13-char line wraps to rows 1-2
    push(vt, "AAAAABBBBBCCC");
    // DD on row 3
    push(vt, "\r\nDD");

    // Screen: SHORT1 / AAAAABBBBB / CCC(cont) / DD
    ASSERT_SCREEN_ROW(vt, screen, 0, "SHORT1");
    ASSERT_SCREEN_ROW(vt, screen, 1, "AAAAABBBBB");
    ASSERT_SCREEN_ROW(vt, screen, 2, "CCC");
    ASSERT_LINEINFO(state, 2, continuation, true);
    ASSERT_SCREEN_ROW(vt, screen, 3, "DD");

    // Scroll SHORT1 into scrollback by adding a line at bottom
    push(vt, "\r\nEE");
    // Screen: AAAAABBBBB / CCC(cont) / DD / EE
    ASSERT_EQ(g_scrollback_count, 1);
    ASSERT_SCREEN_ROW(vt, screen, 0, "AAAAABBBBB");
    ASSERT_SCREEN_ROW(vt, screen, 3, "EE");

    // Resize wider: wrapped line joins, freeing 1 row for scrollback pop
    vt.set_size(4, 15);

    ASSERT_EQ(g_scrollback_count, 0);
    ASSERT_SCREEN_ROW(vt, screen, 0, "SHORT1");
    ASSERT_SCREEN_ROW(vt, screen, 1, "AAAAABBBBBCCC");
    ASSERT_LINEINFO(state, 1, continuation, false);
    ASSERT_SCREEN_ROW(vt, screen, 2, "DD");
    ASSERT_SCREEN_ROW(vt, screen, 3, "EE");
}

// A wrapped line in scrollback rejoins into a single row on resize wider.
//
// Setup: write a 12-char line (wraps to 2 rows at width 8), then more wrapped
// content. Scroll the first wrapped line into scrollback as 2 segments.
// Resize wider -- the scrollback segments rejoin.
TEST(scrollback_reflow_wrapped_line_joins)
{
    REFLOW_SETUP(3, 8);

    // 12-char line wraps to rows 0-1 at width 8
    push(vt, "ABCDEFGHIJKL");
    ASSERT_SCREEN_ROW(vt, screen, 0, "ABCDEFGH");
    ASSERT_SCREEN_ROW(vt, screen, 1, "IJKL");
    ASSERT_LINEINFO(state, 1, continuation, true);

    // Start a new line, then write 12 chars that will wrap
    push(vt, "\r\n");
        // Cursor at (2,0). Now write 12 chars: 8 fit on row 2, then wrap
    // causes scroll, pushing row 0 (ABCDEFGH) to scrollback.
    push(vt, "MNOPQRSTUVWX");

        // Row 0 (ABCDEFGH, no cont) should have scrolled to sb.
    // After further writing, row 1 (IJKL, cont) also scrolls.
    // Let's check what we have on screen and in scrollback

    // Push one more line to ensure both segments are in scrollback
    push(vt, "\r\nZZ");

    ASSERT_EQ(g_scrollback_count, 2);
    // LIFO: [0]=ABCDEFGH (no cont), [1]=IJKL (cont)
    ASSERT_EQ(g_scrollback[0].continuation, false);
    ASSERT_EQ(g_scrollback[1].continuation, true);

        // On screen: wrapped MNOPQRSTUVWX + ZZ. The wrapped part must be on
    // screen for resize to free rows.

        // Resize to 16 cols -- both the on-screen and scrollback wrapped lines
    // should rejoin. The on-screen wrapped line joining frees a row,
    // which gets filled from scrollback.
    vt.set_size(3, 16);

    // Scrollback line ABCDEFGHIJKL (12 chars) fits in 1 row at width 16
    ASSERT_SCREEN_ROW(vt, screen, 0, "ABCDEFGHIJKL");
    ASSERT_LINEINFO(state, 0, continuation, false);
    // On-screen MNOPQRSTUVWX (12 chars) rejoined
    ASSERT_SCREEN_ROW(vt, screen, 1, "MNOPQRSTUVWX");
    ASSERT_LINEINFO(state, 1, continuation, false);
    ASSERT_SCREEN_ROW(vt, screen, 2, "ZZ");
}

// Cursor on the row immediately after a wrapped group. The wrapped group
// must completely fill both of its rows -- this causes old_row to advance
// past old_row_end during the inner reflow copy loop, which requires the
// bounds check (old_row <= old_row_end) to prevent cursor corruption.
//
// Setup: 20-char line fills rows [0,1] completely (10+10). Cursor on row 2.
// One line in scrollback. Resize to 20 cols: group joins to 1 row, freeing
// 1 row for scrollback pop. Cursor must stay on its line.
TEST(scrollback_reflow_cursor_after_reflow)
{
    REFLOW_SETUP(4, 10);

    // SHORT1 scrolls into scrollback
    push(vt, "SHORT1\r\n");
    // 20-char line fills rows 1-2 completely (10+10)
    push(vt, "AAAAABBBBBCCCCCDDDDD\r\n");
    // EE at row 3
    push(vt, "EE");
    // Screen: SHORT1 / AAAAABBBBB / CCCCCDDDDD(cont) / EE
    ASSERT_SCREEN_ROW(vt, screen, 0, "SHORT1");
    ASSERT_SCREEN_ROW(vt, screen, 1, "AAAAABBBBB");
    ASSERT_SCREEN_ROW(vt, screen, 2, "CCCCCDDDDD");
    ASSERT_LINEINFO(state, 2, continuation, true);

    // Scroll SHORT1 off
    push(vt, "\r\nFF");
    // Screen: AAAAABBBBB / CCCCCDDDDD(cont) / EE / FF
    ASSERT_EQ(g_scrollback_count, 1);

    // Move cursor to EE line (row 2, col 1 in 0-based)
    push(vt, "\x1b[3;2H");
    ASSERT_CURSOR(state, 2, 1);

        // Resize to 20 cols: 20-char line joins to 1 row, freeing 1 row.
    // During the join, old_row advances past old_row_end. The cursor
    // at row 2 must NOT be affected by this. Scrollback pops SHORT1.
    vt.set_size(4, 20);

    // Screen: SHORT1 / AAAAABBBBBCCCCCDDDDD / EE / FF
    ASSERT_SCREEN_ROW(vt, screen, 0, "SHORT1");
    ASSERT_SCREEN_ROW(vt, screen, 1, "AAAAABBBBBCCCCCDDDDD");
    ASSERT_SCREEN_ROW(vt, screen, 2, "EE");
    ASSERT_SCREEN_ROW(vt, screen, 3, "FF");

    // Cursor should be on the EE line at col 1
    Pos pos = state.cursor_pos();
    ASSERT_EQ(pos.row, 2);
    ASSERT_EQ(pos.col, 1);
}

// Multiple logical lines -- some wrapped, some not -- pushed into scrollback.
// On resize, they come back in correct order with proper content.
TEST(scrollback_reflow_multiple_logical_lines)
{
    REFLOW_SETUP(4, 10);

    // Write 3 short lines then a wrapped line
    push(vt, "LINE1\r\n"); // row 0
    push(vt, "LINE2\r\n"); // row 1
    push(vt, "PPPPPPPPPPQQ"); // rows 2-3: PPPPPPPPPP / QQ(cont)

    ASSERT_SCREEN_ROW(vt, screen, 0, "LINE1");
    ASSERT_SCREEN_ROW(vt, screen, 3, "QQ");
    ASSERT_LINEINFO(state, 3, continuation, true);

    // Scroll LINE1 and LINE2 into scrollback
    push(vt, "\r\nRR\r\nSS");
    // Screen: PPPPPPPPPP / QQ(cont) / RR / SS
    ASSERT_EQ(g_scrollback_count, 2);

    // Resize wider: wrapped PPPPPPPPPPQQ joins, freeing 1 row
    vt.set_size(4, 15);

    // One scrollback line pops back
    ASSERT_SCREEN_ROW(vt, screen, 0, "LINE2");
    ASSERT_SCREEN_ROW(vt, screen, 1, "PPPPPPPPPPQQ");
    ASSERT_LINEINFO(state, 1, continuation, false);
    ASSERT_SCREEN_ROW(vt, screen, 2, "RR");
    ASSERT_SCREEN_ROW(vt, screen, 3, "SS");

    // LINE1 remains in scrollback (not enough room)
    ASSERT_EQ(g_scrollback_count, 1);
}

// Cursor sits in the continuation segment of a wrapped line.
// After scrollback pop/reflow, cursor must survive correctly.
TEST(scrollback_reflow_cursor_in_continuation)
{
    REFLOW_SETUP(4, 10);

    // SHORT in scrollback
    push(vt, "SHORT\r\n");
    // 13-char wrapped line
    push(vt, "XXXXXYYYYYCCC\r\n");
    // Another line
    push(vt, "ZZ");

    // Screen: SHORT / XXXXXYYYY / CCC(cont) / ZZ
    // Scroll SHORT off
    push(vt, "\r\nWW");
    ASSERT_EQ(g_scrollback_count, 1);

    // Position cursor in the continuation segment (row 1 = CCC)
    push(vt, "\x1b[2;3H"); // row 2 (1-based) = row 1, col 3 (1-based) = col 2
    ASSERT_CURSOR(state, 1, 2);

    // Resize wider
    vt.set_size(4, 15);

    // Verify cursor is valid
    Pos pos = state.cursor_pos();
    ASSERT_TRUE(pos.row >= 0 && pos.row < 4);
    ASSERT_TRUE(pos.col >= 0 && pos.col < 15);

    // Content should be correct
    ASSERT_SCREEN_ROW(vt, screen, 0, "SHORT");
    ASSERT_SCREEN_ROW(vt, screen, 1, "XXXXXYYYYYCCC");
}

// Start narrow (6 cols), write a long line that wraps extensively (18 chars
// = 3 rows). Scroll a short line into scrollback. Resize to 20 cols -- the
// 3-row wrapped line joins into 1, freeing 2 rows. Scrollback pops back.
TEST(scrollback_reflow_narrow_to_wide)
{
    REFLOW_SETUP(4, 6);

    // Short line that will end up in scrollback
    push(vt, "LINE1\r\n");
    // 18-char line wraps to 3 rows at width 6: ABCDEF / GHIJKL / MNOPQR
    push(vt, "ABCDEFGHIJKLMNOPQR");
    ASSERT_SCREEN_ROW(vt, screen, 0, "LINE1");
    ASSERT_SCREEN_ROW(vt, screen, 1, "ABCDEF");
    ASSERT_SCREEN_ROW(vt, screen, 2, "GHIJKL");
    ASSERT_LINEINFO(state, 2, continuation, true);
    ASSERT_SCREEN_ROW(vt, screen, 3, "MNOPQR");
    ASSERT_LINEINFO(state, 3, continuation, true);

    // Scroll LINE1 into scrollback
    push(vt, "\r\nX");
    // Screen: ABCDEF / GHIJKL(cont) / MNOPQR(cont) / X
    ASSERT_EQ(g_scrollback_count, 1);
    ASSERT_SCREEN_ROW(vt, screen, 0, "ABCDEF");

        // Resize to 4 rows, 20 cols.
    // On-screen: 18-char line joins from 3 rows -> 1 row, X stays -> 2 rows.
    // 2 rows free at top. Pop LINE1 from scrollback.
    vt.set_size(4, 20);

    ASSERT_EQ(g_scrollback_count, 0);
    ASSERT_SCREEN_ROW(vt, screen, 0, "LINE1");
    ASSERT_SCREEN_ROW(vt, screen, 1, "ABCDEFGHIJKLMNOPQR");
    ASSERT_LINEINFO(state, 1, continuation, false);
    ASSERT_SCREEN_ROW(vt, screen, 2, "X");
}
