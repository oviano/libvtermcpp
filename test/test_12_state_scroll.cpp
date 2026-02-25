// test_12_state_scroll.cpp — state scrolling tests
// Ported from upstream libvterm t/12state_scroll.test

#include "harness.h"


// Linefeed
TEST(state_scroll_linefeed)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.enable_premove();
    state.reset(true);
    callbacks_clear();

    // Push 24 newlines to get to last row
    push(vt, "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
    ASSERT_CURSOR(state, 24, 0);

    callbacks_clear();

    // One more newline triggers scroll
    push(vt, "\n");
    ASSERT_EQ(g_cb.scrollrect_count, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_row, 0);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_row, 25);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_col, 0);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_col, 80);
    ASSERT_EQ(g_cb.scrollrect[0].downward, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rightward, 0);
    ASSERT_CURSOR(state, 24, 0);
}

// Index
TEST(state_scroll_index)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.enable_premove();
    state.reset(true);
    callbacks_clear();

    push(vt, "\e[25H");
    push(vt, "\eD");
    ASSERT_EQ(g_cb.scrollrect_count, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_row, 0);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_row, 25);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_col, 0);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_col, 80);
    ASSERT_EQ(g_cb.scrollrect[0].downward, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rightward, 0);
}

// Reverse Index
TEST(state_scroll_reverse_index)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.enable_premove();
    state.reset(true);
    callbacks_clear();

    push(vt, "\eM");
    ASSERT_EQ(g_cb.scrollrect_count, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_row, 0);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_row, 25);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_col, 0);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_col, 80);
    ASSERT_EQ(g_cb.scrollrect[0].downward, -1);
    ASSERT_EQ(g_cb.scrollrect[0].rightward, 0);
}

// Linefeed in DECSTBM
TEST(state_scroll_linefeed_in_decstbm)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.enable_premove();
    state.reset(true);
    callbacks_clear();

    push(vt, "\e[1;10r");
    ASSERT_CURSOR(state, 0, 0);

    // Push 9 newlines
    push(vt, "\n\n\n\n\n\n\n\n\n");
    ASSERT_CURSOR(state, 9, 0);

    callbacks_clear();

    push(vt, "\n");
    ASSERT_EQ(g_cb.scrollrect_count, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_row, 0);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_row, 10);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_col, 0);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_col, 80);
    ASSERT_EQ(g_cb.scrollrect[0].downward, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rightward, 0);
    ASSERT_CURSOR(state, 9, 0);
}

// Linefeed outside DECSTBM
TEST(state_scroll_linefeed_outside_decstbm)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.enable_premove();
    state.reset(true);
    callbacks_clear();

    // Set DECSTBM first (needed from prior test context)
    push(vt, "\e[1;10r");

    push(vt, "\e[20H");
    ASSERT_CURSOR(state, 19, 0);

    callbacks_clear();
    push(vt, "\n");
    ASSERT_CURSOR(state, 20, 0);
}

// Index in DECSTBM
TEST(state_scroll_index_in_decstbm)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.enable_premove();
    state.reset(true);
    callbacks_clear();

    push(vt, "\e[9;10r");
    push(vt, "\e[10H");
    push(vt, "\eM");
    ASSERT_CURSOR(state, 8, 0);

    callbacks_clear();
    push(vt, "\eM");
    ASSERT_EQ(g_cb.scrollrect_count, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_row, 8);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_row, 10);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_col, 0);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_col, 80);
    ASSERT_EQ(g_cb.scrollrect[0].downward, -1);
    ASSERT_EQ(g_cb.scrollrect[0].rightward, 0);
}

// Reverse Index in DECSTBM
TEST(state_scroll_reverse_index_in_decstbm)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.enable_premove();
    state.reset(true);
    callbacks_clear();

    // Set DECSTBM to 9;10
    push(vt, "\e[9;10r");

    push(vt, "\e[25H");
    ASSERT_CURSOR(state, 24, 0);

    callbacks_clear();
    push(vt, "\n");
    // no scrollrect - cursor is outside the scroll region bottom
    ASSERT_EQ(g_cb.scrollrect_count, 0);
    ASSERT_CURSOR(state, 24, 0);
}

// Linefeed in DECSTBM+DECSLRM
TEST(state_scroll_linefeed_in_decstbm_decslrm)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.enable_premove();
    state.reset(true);
    callbacks_clear();

    push(vt, "\e[?69h");
    push(vt, "\e[3;10r\e[10;40s");

    callbacks_clear();
    push(vt, "\e[10;10H\n");
    ASSERT_EQ(g_cb.scrollrect_count, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_row, 2);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_row, 10);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_col, 9);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_col, 40);
    ASSERT_EQ(g_cb.scrollrect[0].downward, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rightward, 0);
}

// IND/RI in DECSTBM+DECSLRM
TEST(state_scroll_ind_ri_in_decstbm_decslrm)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.enable_premove();
    state.reset(true);
    callbacks_clear();

    push(vt, "\e[?69h");
    push(vt, "\e[3;10r\e[10;40s");
    // Move to bottom of scroll region
    push(vt, "\e[10;10H");

    callbacks_clear();
    push(vt, "\eD");
    ASSERT_EQ(g_cb.scrollrect_count, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_row, 2);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_row, 10);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_col, 9);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_col, 40);
    ASSERT_EQ(g_cb.scrollrect[0].downward, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rightward, 0);

    callbacks_clear();
    push(vt, "\e[3;10H\eM");
    ASSERT_EQ(g_cb.scrollrect_count, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_row, 2);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_row, 10);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_col, 9);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_col, 40);
    ASSERT_EQ(g_cb.scrollrect[0].downward, -1);
    ASSERT_EQ(g_cb.scrollrect[0].rightward, 0);
}

// DECRQSS on DECSTBM
TEST(state_scroll_decrqss_decstbm)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.enable_premove();
    state.reset(true);
    output_init(vt);
    callbacks_clear();

    push(vt, "\e[?69h");
    push(vt, "\e[3;10r\e[10;40s");

    output_clear();
    push(vt, "\eP$qr\e\\");
    ASSERT_OUTPUT_BYTES("\eP1$r3;10r\e\\", 12);
}

// DECRQSS on DECSLRM
TEST(state_scroll_decrqss_decslrm)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.enable_premove();
    state.reset(true);
    output_init(vt);
    callbacks_clear();

    push(vt, "\e[?69h");
    push(vt, "\e[3;10r\e[10;40s");

    output_clear();
    push(vt, "\eP$qs\e\\");
    ASSERT_OUTPUT_BYTES("\eP1$r10;40s\e\\", 13);
}

// Setting invalid DECSLRM with !DECVSSM is still rejected
TEST(state_scroll_invalid_decslrm_without_decvssm)
{
    // no assertions — verifying no crash
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.enable_premove();
    state.reset(true);
    callbacks_clear();

    // Disable DECVSSM (mode 69), attempt invalid DECSLRM, then re-enable
    push(vt, "\e[?69l\e[;0s\e[?69h");
    // Just ensuring no crash; no specific assertions from upstream
}

// Scroll Down
TEST(state_scroll_down)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.enable_premove();
    state.reset(true);
    callbacks_clear();

    push(vt, "\e[S");
    ASSERT_EQ(g_cb.scrollrect_count, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_row, 0);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_row, 25);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_col, 0);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_col, 80);
    ASSERT_EQ(g_cb.scrollrect[0].downward, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rightward, 0);
    ASSERT_CURSOR(state, 0, 0);

    callbacks_clear();
    push(vt, "\e[2S");
    ASSERT_EQ(g_cb.scrollrect_count, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_row, 0);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_row, 25);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_col, 0);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_col, 80);
    ASSERT_EQ(g_cb.scrollrect[0].downward, 2);
    ASSERT_EQ(g_cb.scrollrect[0].rightward, 0);
    ASSERT_CURSOR(state, 0, 0);

    callbacks_clear();
    push(vt, "\e[100S");
    ASSERT_EQ(g_cb.scrollrect_count, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_row, 0);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_row, 25);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_col, 0);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_col, 80);
    ASSERT_EQ(g_cb.scrollrect[0].downward, 25);
    ASSERT_EQ(g_cb.scrollrect[0].rightward, 0);
}

// Scroll Up
TEST(state_scroll_up)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.enable_premove();
    state.reset(true);
    callbacks_clear();

    push(vt, "\e[T");
    ASSERT_EQ(g_cb.scrollrect_count, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_row, 0);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_row, 25);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_col, 0);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_col, 80);
    ASSERT_EQ(g_cb.scrollrect[0].downward, -1);
    ASSERT_EQ(g_cb.scrollrect[0].rightward, 0);
    ASSERT_CURSOR(state, 0, 0);

    callbacks_clear();
    push(vt, "\e[2T");
    ASSERT_EQ(g_cb.scrollrect_count, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_row, 0);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_row, 25);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_col, 0);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_col, 80);
    ASSERT_EQ(g_cb.scrollrect[0].downward, -2);
    ASSERT_EQ(g_cb.scrollrect[0].rightward, 0);
    ASSERT_CURSOR(state, 0, 0);

    callbacks_clear();
    push(vt, "\e[100T");
    ASSERT_EQ(g_cb.scrollrect_count, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_row, 0);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_row, 25);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_col, 0);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_col, 80);
    ASSERT_EQ(g_cb.scrollrect[0].downward, -25);
    ASSERT_EQ(g_cb.scrollrect[0].rightward, 0);
}

// SD/SU in DECSTBM
TEST(state_scroll_sd_su_in_decstbm)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.enable_premove();
    state.reset(true);
    callbacks_clear();

    push(vt, "\e[5;20r");

    callbacks_clear();
    push(vt, "\e[S");
    ASSERT_EQ(g_cb.scrollrect_count, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_row, 4);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_row, 20);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_col, 0);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_col, 80);
    ASSERT_EQ(g_cb.scrollrect[0].downward, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rightward, 0);

    callbacks_clear();
    push(vt, "\e[T");
    ASSERT_EQ(g_cb.scrollrect_count, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_row, 4);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_row, 20);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_col, 0);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_col, 80);
    ASSERT_EQ(g_cb.scrollrect[0].downward, -1);
    ASSERT_EQ(g_cb.scrollrect[0].rightward, 0);
}

// SD/SU in DECSTBM+DECSLRM
TEST(state_scroll_sd_su_in_decstbm_decslrm)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.enable_premove();
    state.reset(true);
    callbacks_clear();

    push(vt, "\e[?69h");
    push(vt, "\e[3;10r\e[10;40s");
    ASSERT_CURSOR(state, 0, 0);

    push(vt, "\e[3;10H");
    ASSERT_CURSOR(state, 2, 9);

    callbacks_clear();
    push(vt, "\e[S");
    ASSERT_EQ(g_cb.scrollrect_count, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_row, 2);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_row, 10);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_col, 9);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_col, 40);
    ASSERT_EQ(g_cb.scrollrect[0].downward, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rightward, 0);

    // Disable DECSLRM
    push(vt, "\e[?69l");

    callbacks_clear();
    push(vt, "\e[S");
    ASSERT_EQ(g_cb.scrollrect_count, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_row, 2);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_row, 10);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_col, 0);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_col, 80);
    ASSERT_EQ(g_cb.scrollrect[0].downward, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rightward, 0);
}

// Invalid boundaries
TEST(state_scroll_invalid_boundaries)
{
    // no assertions — verifying no crash
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.enable_premove();
    state.reset(true);
    callbacks_clear();

    // These should not crash; invalid scroll regions are silently ignored
    push(vt, "\e[100;105r\eD");
    push(vt, "\e[5;2r\eD");
}

// Scroll Down move+erase emulation
TEST(state_scroll_down_moverect_erase_emulation)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs_no_scrollrect);
    state.enable_premove();
    state.reset(true);
    callbacks_clear();

    push(vt, "\e[S");
    // premove 0..1,0..80
    ASSERT_EQ(g_cb.premove_count, 1);
    ASSERT_EQ(g_cb.premove[0].rect.start_row, 0);
    ASSERT_EQ(g_cb.premove[0].rect.end_row, 1);
    ASSERT_EQ(g_cb.premove[0].rect.start_col, 0);
    ASSERT_EQ(g_cb.premove[0].rect.end_col, 80);
    // moverect 1..25,0..80 -> 0..24,0..80
    ASSERT_EQ(g_cb.moverect_count, 1);
    ASSERT_EQ(g_cb.moverect[0].src.start_row, 1);
    ASSERT_EQ(g_cb.moverect[0].src.end_row, 25);
    ASSERT_EQ(g_cb.moverect[0].src.start_col, 0);
    ASSERT_EQ(g_cb.moverect[0].src.end_col, 80);
    ASSERT_EQ(g_cb.moverect[0].dest.start_row, 0);
    ASSERT_EQ(g_cb.moverect[0].dest.end_row, 24);
    ASSERT_EQ(g_cb.moverect[0].dest.start_col, 0);
    ASSERT_EQ(g_cb.moverect[0].dest.end_col, 80);
    // erase 24..25,0..80
    ASSERT_EQ(g_cb.erase_count, 1);
    ASSERT_EQ(g_cb.erase[0].rect.start_row, 24);
    ASSERT_EQ(g_cb.erase[0].rect.end_row, 25);
    ASSERT_EQ(g_cb.erase[0].rect.start_col, 0);
    ASSERT_EQ(g_cb.erase[0].rect.end_col, 80);
    ASSERT_CURSOR(state, 0, 0);

    callbacks_clear();
    push(vt, "\e[2S");
    // premove 0..2,0..80
    ASSERT_EQ(g_cb.premove_count, 1);
    ASSERT_EQ(g_cb.premove[0].rect.start_row, 0);
    ASSERT_EQ(g_cb.premove[0].rect.end_row, 2);
    ASSERT_EQ(g_cb.premove[0].rect.start_col, 0);
    ASSERT_EQ(g_cb.premove[0].rect.end_col, 80);
    // moverect 2..25,0..80 -> 0..23,0..80
    ASSERT_EQ(g_cb.moverect_count, 1);
    ASSERT_EQ(g_cb.moverect[0].src.start_row, 2);
    ASSERT_EQ(g_cb.moverect[0].src.end_row, 25);
    ASSERT_EQ(g_cb.moverect[0].src.start_col, 0);
    ASSERT_EQ(g_cb.moverect[0].src.end_col, 80);
    ASSERT_EQ(g_cb.moverect[0].dest.start_row, 0);
    ASSERT_EQ(g_cb.moverect[0].dest.end_row, 23);
    ASSERT_EQ(g_cb.moverect[0].dest.start_col, 0);
    ASSERT_EQ(g_cb.moverect[0].dest.end_col, 80);
    // erase 23..25,0..80
    ASSERT_EQ(g_cb.erase_count, 1);
    ASSERT_EQ(g_cb.erase[0].rect.start_row, 23);
    ASSERT_EQ(g_cb.erase[0].rect.end_row, 25);
    ASSERT_EQ(g_cb.erase[0].rect.start_col, 0);
    ASSERT_EQ(g_cb.erase[0].rect.end_col, 80);
    ASSERT_CURSOR(state, 0, 0);
}

// Scroll Up move+erase emulation
TEST(state_scroll_up_moverect_erase_emulation)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs_no_scrollrect);
    state.enable_premove();
    state.reset(true);
    callbacks_clear();

    push(vt, "\e[T");
    // premove 24..25,0..80
    ASSERT_EQ(g_cb.premove_count, 1);
    ASSERT_EQ(g_cb.premove[0].rect.start_row, 24);
    ASSERT_EQ(g_cb.premove[0].rect.end_row, 25);
    ASSERT_EQ(g_cb.premove[0].rect.start_col, 0);
    ASSERT_EQ(g_cb.premove[0].rect.end_col, 80);
    // moverect 0..24,0..80 -> 1..25,0..80
    ASSERT_EQ(g_cb.moverect_count, 1);
    ASSERT_EQ(g_cb.moverect[0].src.start_row, 0);
    ASSERT_EQ(g_cb.moverect[0].src.end_row, 24);
    ASSERT_EQ(g_cb.moverect[0].src.start_col, 0);
    ASSERT_EQ(g_cb.moverect[0].src.end_col, 80);
    ASSERT_EQ(g_cb.moverect[0].dest.start_row, 1);
    ASSERT_EQ(g_cb.moverect[0].dest.end_row, 25);
    ASSERT_EQ(g_cb.moverect[0].dest.start_col, 0);
    ASSERT_EQ(g_cb.moverect[0].dest.end_col, 80);
    // erase 0..1,0..80
    ASSERT_EQ(g_cb.erase_count, 1);
    ASSERT_EQ(g_cb.erase[0].rect.start_row, 0);
    ASSERT_EQ(g_cb.erase[0].rect.end_row, 1);
    ASSERT_EQ(g_cb.erase[0].rect.start_col, 0);
    ASSERT_EQ(g_cb.erase[0].rect.end_col, 80);
    ASSERT_CURSOR(state, 0, 0);

    callbacks_clear();
    push(vt, "\e[2T");
    // premove 23..25,0..80
    ASSERT_EQ(g_cb.premove_count, 1);
    ASSERT_EQ(g_cb.premove[0].rect.start_row, 23);
    ASSERT_EQ(g_cb.premove[0].rect.end_row, 25);
    ASSERT_EQ(g_cb.premove[0].rect.start_col, 0);
    ASSERT_EQ(g_cb.premove[0].rect.end_col, 80);
    // moverect 0..23,0..80 -> 2..25,0..80
    ASSERT_EQ(g_cb.moverect_count, 1);
    ASSERT_EQ(g_cb.moverect[0].src.start_row, 0);
    ASSERT_EQ(g_cb.moverect[0].src.end_row, 23);
    ASSERT_EQ(g_cb.moverect[0].src.start_col, 0);
    ASSERT_EQ(g_cb.moverect[0].src.end_col, 80);
    ASSERT_EQ(g_cb.moverect[0].dest.start_row, 2);
    ASSERT_EQ(g_cb.moverect[0].dest.end_row, 25);
    ASSERT_EQ(g_cb.moverect[0].dest.start_col, 0);
    ASSERT_EQ(g_cb.moverect[0].dest.end_col, 80);
    // erase 0..2,0..80
    ASSERT_EQ(g_cb.erase_count, 1);
    ASSERT_EQ(g_cb.erase[0].rect.start_row, 0);
    ASSERT_EQ(g_cb.erase[0].rect.end_row, 2);
    ASSERT_EQ(g_cb.erase[0].rect.start_col, 0);
    ASSERT_EQ(g_cb.erase[0].rect.end_col, 80);
    ASSERT_CURSOR(state, 0, 0);
}

// DECSTBM resets cursor position
TEST(state_scroll_decstbm_resets_cursor)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.enable_premove();
    state.reset(true);
    callbacks_clear();

    push(vt, "\e[5;5H");
    ASSERT_CURSOR(state, 4, 4);

    push(vt, "\e[r");
    ASSERT_CURSOR(state, 0, 0);
}
