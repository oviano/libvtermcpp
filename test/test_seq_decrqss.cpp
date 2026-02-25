// test_seq_decrqss.cpp -- per-sequence tests for DECRQSS (DCS $ q)
//
// DECRQSS (Request Status String) queries the current state of various
// terminal settings via DCS:
//   DCS $ q <selector> ST
//
// Response format:
//   DCS 1 $ r <value> ST   -- valid request (1 = valid)
//   DCS 0 $ r ST            -- invalid/unknown request (0 = invalid)
//
// Selectors:
//   m      -- SGR (Select Graphic Rendition)
//   r      -- DECSTBM (scroll region top/bottom margins)
//   SP q   -- DECSCUSR (cursor style)

#include "harness.h"

// ============================================================================
// DECRQSS for SGR
// ============================================================================

// Set SGR 1;5;7 (bold, blink, reverse), query with DECRQSS, verify response
TEST(seq_decrqss_sgr)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    output_init(vt);
    state.reset(true);
    callbacks_clear();
    output_clear();

    // Set bold, blink, reverse
    push(vt, "\e[1;5;7m");
    output_clear();

    // Query SGR: DCS $ q m ST
    push(vt, "\eP$qm\e\\");
    // Response: DCS 1 $ r 1;5;7 m ST = \eP1$r1;5;7m\e\\ (13 bytes)
    ASSERT_OUTPUT_BYTES("\eP1$r1;5;7m\e\\", 13);
}

// ============================================================================
// DECRQSS for DECSTBM
// ============================================================================

// Set DECSTBM 5;20, query, verify response
TEST(seq_decrqss_decstbm)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    output_init(vt);
    state.reset(true);
    callbacks_clear();
    output_clear();

    // Set scroll region to rows 5..20 (1-based)
    push(vt, "\e[5;20r");
    output_clear();

    // Query DECSTBM: DCS $ q r ST
    push(vt, "\eP$qr\e\\");
    // Response: DCS 1 $ r 5;20 r ST = \eP1$r5;20r\e\\ (12 bytes)
    ASSERT_OUTPUT_BYTES("\eP1$r5;20r\e\\", 12);
}

// ============================================================================
// DECRQSS for DECSCUSR
// ============================================================================

// Set cursor to underline blink (CSI 3 SP q), query DECSCUSR
TEST(seq_decrqss_decscusr)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    output_init(vt);
    state.reset(true);
    callbacks_clear();
    output_clear();

    // Set cursor shape to underline blink (3 = underline, blink)
    push(vt, "\e[3 q");
    output_clear();

    // Query DECSCUSR: DCS $ q SP q ST
    push(vt, "\eP$q q\e\\");
    // Response: DCS 1 $ r 3 SP q ST = \eP1$r3 q\e\\ (10 bytes)
    ASSERT_OUTPUT_BYTES("\eP1$r3 q\e\\", 10);
}

// ============================================================================
// DECRQSS for unknown attribute
// ============================================================================

// Query an unrecognised attribute, verify response indicates invalid
TEST(seq_decrqss_unknown)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    output_init(vt);
    state.reset(true);
    callbacks_clear();
    output_clear();

    // Query unknown: DCS $ q z ST
    push(vt, "\eP$qz\e\\");
    // Response: DCS 0 $ r ST = \eP0$r\e\\ (7 bytes)
    ASSERT_OUTPUT_BYTES("\eP0$r\e\\", 7);
}
