// test_seq_sgr_color.cpp -- tests for SGR color attributes
//
// Tests SGR (Select Graphic Rendition) sequences for setting foreground
// and background colors via the standard 16-color palette, high-intensity
// palette, 256-color indexed mode, RGB direct color, and default resets.
//
// SGR 30-37  -> foreground palette indices 0-7
// SGR 40-47  -> background palette indices 0-7
// SGR 90-97  -> foreground high-intensity indices 8-15
// SGR 100-107 -> background high-intensity indices 8-15
// SGR 38;5;N -> foreground 256-color indexed
// SGR 48;5;N -> background 256-color indexed
// SGR 38;2;R;G;B   -> foreground RGB (semicolon form)
// SGR 38:2::R:G:B  -> foreground RGB (colon form with colorspace)
// SGR 39    -> default foreground
// SGR 49    -> default background

#include "harness.h"

// ============================================================================
// 16-color palette
// ============================================================================

// SGR 31 sets foreground to index 1 (red), SGR 37 to index 7 (white)
TEST(seq_sgr_fg_palette_16)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    push(vt, "\e[31m");
    ASSERT_PEN_COLOR_IDX(state, Attr::Foreground, 1);

    push(vt, "\e[37m");
    ASSERT_PEN_COLOR_IDX(state, Attr::Foreground, 7);
}

// SGR 41 sets background to index 1
TEST(seq_sgr_bg_palette_16)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    push(vt, "\e[41m");
    ASSERT_PEN_COLOR_IDX(state, Attr::Background, 1);
}

// ============================================================================
// High-intensity colors
// ============================================================================

// SGR 91 sets foreground to index 9, SGR 97 to index 15
TEST(seq_sgr_fg_high_intensity)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    push(vt, "\e[91m");
    ASSERT_PEN_COLOR_IDX(state, Attr::Foreground, 9);

    push(vt, "\e[97m");
    ASSERT_PEN_COLOR_IDX(state, Attr::Foreground, 15);
}

// SGR 101 sets background to index 9
TEST(seq_sgr_bg_high_intensity)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    push(vt, "\e[101m");
    ASSERT_PEN_COLOR_IDX(state, Attr::Background, 9);
}

// ============================================================================
// Default color resets
// ============================================================================

// SGR 39 resets foreground to default (RGB 240,240,240)
TEST(seq_sgr_fg_default)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    push(vt, "\e[31m");
    ASSERT_PEN_COLOR_IDX(state, Attr::Foreground, 1);

    push(vt, "\e[39m");
    ASSERT_PEN_COLOR_DEFAULT_FG(state, Attr::Foreground, 240, 240, 240);
}

// SGR 49 resets background to default (RGB 0,0,0)
TEST(seq_sgr_bg_default)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    push(vt, "\e[41m");
    ASSERT_PEN_COLOR_IDX(state, Attr::Background, 1);

    push(vt, "\e[49m");
    ASSERT_PEN_COLOR_DEFAULT_BG(state, Attr::Background, 0, 0, 0);
}

// ============================================================================
// 256-color indexed mode
// ============================================================================

// SGR 38;5;196 sets foreground to palette index 196
TEST(seq_sgr_fg_256_indexed)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    push(vt, "\e[38;5;196m");
    ASSERT_PEN_COLOR_IDX(state, Attr::Foreground, 196);
}

// SGR 48;5;52 sets background to palette index 52
TEST(seq_sgr_bg_256_indexed)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    push(vt, "\e[48;5;52m");
    ASSERT_PEN_COLOR_IDX(state, Attr::Background, 52);
}

// ============================================================================
// RGB direct color
// ============================================================================

// SGR 38:2:R:G:B -- colon form (ITU T.416 without colorspace ID)
TEST(seq_sgr_fg_rgb_colon)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    push(vt, "\e[38:2:255:128:64m");
    ASSERT_PEN_COLOR_RGB(state, Attr::Foreground, 255, 128, 64);
}

// SGR 38;2;R;G;B -- semicolon form (widely used by applications)
TEST(seq_sgr_fg_rgb_semicolon)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    push(vt, "\e[38;2;100;200;50m");
    ASSERT_PEN_COLOR_RGB(state, Attr::Foreground, 100, 200, 50);
}
