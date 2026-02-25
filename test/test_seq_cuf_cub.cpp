// test_seq_cuf_cub.cpp — per-sequence tests for CUF (CSI C) and CUB (CSI D)
//
// CUF (Cursor Forward):  ESC [ <n> C   — move cursor right by <n> columns
// CUB (Cursor Backward): ESC [ <n> D   — move cursor left by <n> columns
//
// Tests cover default params, explicit params, zero-as-one behaviour,
// clamping at screen edges, and margin clamping under DECLRMM (mode 69).

#include "harness.h"

// ============================================================================
// CUF — Cursor Forward (CSI C)
// ============================================================================

// CUF with no parameter moves the cursor right by 1 column
TEST(seq_cuf_default_param)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Place cursor at row 0, col 5 (CSI 1;6 H — 1-based)
    push(vt, "\e[1;6H");
    ASSERT_CURSOR(state, 0, 5);

    // CUF with no param: move right 1
    push(vt, "\e[C");
    ASSERT_CURSOR(state, 0, 6);
}

// CUF with an explicit count moves the cursor right by that many columns
TEST(seq_cuf_explicit_param)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Place cursor at row 3, col 10 (CSI 4;11 H)
    push(vt, "\e[4;11H");
    ASSERT_CURSOR(state, 3, 10);

    // CUF 5: move right 5
    push(vt, "\e[5C");
    ASSERT_CURSOR(state, 3, 15);
}

// CUF with parameter 0 is treated as 1 (DEC spec)
TEST(seq_cuf_zero_param)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Place cursor at row 0, col 10
    push(vt, "\e[1;11H");
    ASSERT_CURSOR(state, 0, 10);

    // CUF 0: treated as CUF 1
    push(vt, "\e[0C");
    ASSERT_CURSOR(state, 0, 11);
}

// CUF at the right edge of the screen stays clamped at col 79
TEST(seq_cuf_clamp_right_edge)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Place cursor at the last column: row 0, col 79 (CSI 1;80 H)
    push(vt, "\e[1;80H");
    ASSERT_CURSOR(state, 0, 79);

    // CUF should not move past col 79
    push(vt, "\e[C");
    ASSERT_CURSOR(state, 0, 79);

    // Even with a large count
    push(vt, "\e[999C");
    ASSERT_CURSOR(state, 0, 79);
}

// CUF respects right margin when DECLRMM (mode 69) is active and the cursor
// is inside the margin region.  Origin mode (DECOM) must also be enabled so
// that the post-CSI clamping uses the scroll-region bounds.
TEST(seq_cuf_clamp_with_declrmm)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Enable DECLRMM (left/right margin mode)
    push(vt, "\e[?69h");

    // Set left margin 10, right margin 50 (DECSLRM, 1-based)
    push(vt, "\e[10;50s");

    // Enable origin mode so cursor is relative to and bounded by margins
    push(vt, "\e[?6h");

        // In origin mode, CSI H homes to the top-left of the scroll region.
    // Move right a bit so we start inside the region.
    push(vt, "\e[H");
    // Origin-mode home = top-left of scroll region = row 0, col 9 (0-based)
    ASSERT_CURSOR(state, 0, 9);

    // CUF 5: should move to col 14 (0-based)
    push(vt, "\e[5C");
    ASSERT_CURSOR(state, 0, 14);

    // CUF with a large count should stop at right margin (col 49, 0-based)
    push(vt, "\e[999C");
    ASSERT_CURSOR(state, 0, 49);
}

// ============================================================================
// CUB — Cursor Backward (CSI D)
// ============================================================================

// CUB with no parameter moves the cursor left by 1 column
TEST(seq_cub_default_param)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Place cursor at row 0, col 5
    push(vt, "\e[1;6H");
    ASSERT_CURSOR(state, 0, 5);

    // CUB with no param: move left 1
    push(vt, "\e[D");
    ASSERT_CURSOR(state, 0, 4);
}

// CUB at the left edge of the screen stays clamped at col 0
TEST(seq_cub_clamp_left_edge)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Cursor starts at row 0, col 0 after reset
    ASSERT_CURSOR(state, 0, 0);

    // CUB should not move past col 0
    push(vt, "\e[D");
    ASSERT_CURSOR(state, 0, 0);

    // Even with a large count
    push(vt, "\e[999D");
    ASSERT_CURSOR(state, 0, 0);
}

// CUB respects left margin when DECLRMM (mode 69) is active and the cursor
// is inside the margin region.  Origin mode (DECOM) must also be enabled so
// that the post-CSI clamping uses the scroll-region bounds.
TEST(seq_cub_clamp_with_declrmm)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Enable DECLRMM (left/right margin mode)
    push(vt, "\e[?69h");

    // Set left margin 10, right margin 50 (DECSLRM, 1-based)
    push(vt, "\e[10;50s");

    // Enable origin mode so cursor is relative to and bounded by margins
    push(vt, "\e[?6h");

        // Move cursor to the right side of the margin region.
    // In origin mode, CSI 1;40 H = row 0 + top, col 39 + left of region.
    // Absolute position = row 0, col 9 + 39 = col 48 (0-based).
    push(vt, "\e[1;40H");
    ASSERT_CURSOR(state, 0, 48);

    // CUB 5: should move left to col 43 (0-based)
    push(vt, "\e[5D");
    ASSERT_CURSOR(state, 0, 43);

    // CUB with a large count should stop at left margin (col 9, 0-based)
    push(vt, "\e[999D");
    ASSERT_CURSOR(state, 0, 9);
}
