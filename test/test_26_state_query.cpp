// test_26_state_query.cpp — state query tests
// Ported from upstream libvterm t/26state_query.test

#include "harness.h"

// DA
TEST(state_query_da)
{
    Terminal vt(25, 80);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    output_init(vt);

    state.reset(true);
    callbacks_clear();
    output_clear();

    push(vt, "\e[c");
    ASSERT_OUTPUT_BYTES("\e[?1;2c", 7);
}

// XTVERSION
TEST(state_query_xtversion)
{
    Terminal vt(25, 80);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    output_init(vt);

    state.reset(true);
    callbacks_clear();
    output_clear();

    push(vt, "\e[>q");
    ASSERT_OUTPUT_BYTES("\eP>|libvterm(0.3)\e\\", 19);
}

// DSR
TEST(state_query_dsr)
{
    Terminal vt(25, 80);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    output_init(vt);

    state.reset(true);
    callbacks_clear();
    output_clear();

    push(vt, "\e[5n");
    ASSERT_OUTPUT_BYTES("\e[0n", 4);
}

// CPR
TEST(state_query_cpr)
{
    Terminal vt(25, 80);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    output_init(vt);

    state.reset(true);
    callbacks_clear();
    output_clear();

    push(vt, "\e[6n");
    ASSERT_OUTPUT_BYTES("\e[1;1R", 6);

    output_clear();
    push(vt, "\e[10;10H\e[6n");
    ASSERT_OUTPUT_BYTES("\e[10;10R", 8);
}

// DECCPR
TEST(state_query_deccpr)
{
    Terminal vt(25, 80);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    output_init(vt);

    state.reset(true);
    callbacks_clear();
    output_clear();

        // Cursor should still be at 10,10 from CPR test? No, fresh vterm.
    // The upstream test continues from CPR without RESET, so cursor is at 10,10.
    // But each TEST() here gets its own VTerm, so we must move cursor first.
    push(vt, "\e[10;10H");
    output_clear();

    push(vt, "\e[?6n");
    ASSERT_OUTPUT_BYTES("\e[?10;10R", 9);
}

// DECRQSS on DECSCUSR
TEST(state_query_decrqss_decscusr)
{
    Terminal vt(25, 80);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    output_init(vt);

    state.reset(true);
    callbacks_clear();
    output_clear();

    push(vt, "\e[3 q");
    output_clear();
    push(vt, "\eP$q q\e\\");
    ASSERT_OUTPUT_BYTES("\eP1$r3 q\e\\", 10);
}

// DECRQSS on SGR
TEST(state_query_decrqss_sgr)
{
    Terminal vt(25, 80);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    output_init(vt);

    state.reset(true);
    callbacks_clear();
    output_clear();

    push(vt, "\e[1;5;7m");
    output_clear();
    push(vt, "\eP$qm\e\\");
    ASSERT_OUTPUT_BYTES("\eP1$r1;5;7m\e\\", 13);
}

// DECRQSS on SGR ANSI colours
TEST(state_query_decrqss_sgr_ansi_colours)
{
    Terminal vt(25, 80);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    output_init(vt);

    state.reset(true);
    callbacks_clear();
    output_clear();

    push(vt, "\e[0;31;42m");
    output_clear();
    push(vt, "\eP$qm\e\\");
    ASSERT_OUTPUT_BYTES("\eP1$r31;42m\e\\", 13);
}

// DECRQSS on SGR ANSI hi-bright colours
TEST(state_query_decrqss_sgr_ansi_hibright_colours)
{
    Terminal vt(25, 80);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    output_init(vt);

    state.reset(true);
    callbacks_clear();
    output_clear();

    push(vt, "\e[0;93;104m");
    output_clear();
    push(vt, "\eP$qm\e\\");
    ASSERT_OUTPUT_BYTES("\eP1$r93;104m\e\\", 14);
}

// DECRQSS on SGR 256-palette colours
TEST(state_query_decrqss_sgr_256_palette_colours)
{
    Terminal vt(25, 80);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    output_init(vt);

    state.reset(true);
    callbacks_clear();
    output_clear();

    push(vt, "\e[0;38:5:56;48:5:78m");
    output_clear();
    push(vt, "\eP$qm\e\\");
    ASSERT_OUTPUT_BYTES("\eP1$r38:5:56;48:5:78m\e\\", 23);
}

// DECRQSS on SGR RGB8 colours
TEST(state_query_decrqss_sgr_rgb8_colours)
{
    Terminal vt(25, 80);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    output_init(vt);

    state.reset(true);
    callbacks_clear();
    output_clear();

    push(vt, "\e[0;38:2:24:68:112;48:2:13:57:101m");
    output_clear();
    push(vt, "\eP$qm\e\\");
    ASSERT_OUTPUT_BYTES("\eP1$r38:2:24:68:112;48:2:13:57:101m\e\\", 37);
}

// S8C1T on DSR
TEST(state_query_s8c1t_dsr)
{
    Terminal vt(25, 80);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    output_init(vt);

    state.reset(true);
    callbacks_clear();
    output_clear();

    // Enable S8C1T: ESC SP G
    push(vt, "\e G");
    output_clear();

    push(vt, "\e[5n");
    // output "\x9b" "0n" — CSI as single 8-bit C1 byte
    ASSERT_OUTPUT_BYTES("\x9b" "0n", 3);

    // Disable S8C1T: ESC SP F
    push(vt, "\e F");
}
