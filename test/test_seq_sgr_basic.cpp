// test_seq_sgr_basic.cpp -- tests for SGR basic attributes (CSI m, codes 0-9, 21-29)
//
// SGR -- Select Graphic Rendition (CSI Ps m)
//   Ps = 0   Reset all attributes
//   Ps = 1   Bold on                  Ps = 22  Bold off
//   Ps = 3   Italic on                Ps = 23  Italic off
//   Ps = 4   Underline single         Ps = 24  Underline off
//   Ps = 5   Blink on                 Ps = 25  Blink off
//   Ps = 7   Reverse on               Ps = 27  Reverse off
//   Ps = 8   Conceal on               Ps = 28  Conceal off
//   Ps = 9   Strikethrough on         Ps = 29  Strikethrough off
//   Ps = 10  Default font (font 0)
//   Ps = 11  Alternate font 1
//   Ps = 21  Underline double

#include "harness.h"

// ============================================================================
// SGR 0 -- Reset
// ============================================================================

// SGR 0 resets all attributes to their defaults
TEST(seq_sgr_reset)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Set bold on
    push(vt, "\e[1m");
    ASSERT_PEN_BOOL(state, Attr::Bold, true);

    // Reset all attributes
    push(vt, "\e[0m");
    ASSERT_PEN_BOOL(state, Attr::Bold, false);
}

// ============================================================================
// SGR 1/22 -- Bold on/off
// ============================================================================

// SGR 1 enables bold, SGR 22 disables it
TEST(seq_sgr_bold_on_off)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Bold on
    push(vt, "\e[1m");
    ASSERT_PEN_BOOL(state, Attr::Bold, true);

    // Bold off
    push(vt, "\e[22m");
    ASSERT_PEN_BOOL(state, Attr::Bold, false);
}

// ============================================================================
// SGR 3/23 -- Italic on/off
// ============================================================================

// SGR 3 enables italic, SGR 23 disables it
TEST(seq_sgr_italic_on_off)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Italic on
    push(vt, "\e[3m");
    ASSERT_PEN_BOOL(state, Attr::Italic, true);

    // Italic off
    push(vt, "\e[23m");
    ASSERT_PEN_BOOL(state, Attr::Italic, false);
}

// ============================================================================
// SGR 4/24 -- Underline single on/off
// ============================================================================

// SGR 4 sets underline to single, SGR 24 turns it off
TEST(seq_sgr_underline_on_off)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Underline single
    push(vt, "\e[4m");
    ASSERT_PEN_INT(state, Attr::Underline, to_underlying(Underline::Single));

    // Underline off
    push(vt, "\e[24m");
    ASSERT_PEN_INT(state, Attr::Underline, to_underlying(Underline::Off));
}

// ============================================================================
// SGR 21 -- Underline double
// ============================================================================

// SGR 21 sets underline to double
TEST(seq_sgr_underline_double)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Underline double
    push(vt, "\e[21m");
    ASSERT_PEN_INT(state, Attr::Underline, to_underlying(Underline::Double));
}

// ============================================================================
// SGR 5/25 -- Blink on/off
// ============================================================================

// SGR 5 enables blink, SGR 25 disables it
TEST(seq_sgr_blink_on_off)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Blink on
    push(vt, "\e[5m");
    ASSERT_PEN_BOOL(state, Attr::Blink, true);

    // Blink off
    push(vt, "\e[25m");
    ASSERT_PEN_BOOL(state, Attr::Blink, false);
}

// ============================================================================
// SGR 7/27 -- Reverse on/off
// ============================================================================

// SGR 7 enables reverse video, SGR 27 disables it
TEST(seq_sgr_reverse_on_off)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Reverse on
    push(vt, "\e[7m");
    ASSERT_PEN_BOOL(state, Attr::Reverse, true);

    // Reverse off
    push(vt, "\e[27m");
    ASSERT_PEN_BOOL(state, Attr::Reverse, false);
}

// ============================================================================
// SGR 8/28 -- Conceal on/off
// ============================================================================

// SGR 8 enables conceal (invisible), SGR 28 disables it
TEST(seq_sgr_conceal_on_off)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Conceal on
    push(vt, "\e[8m");
    ASSERT_PEN_BOOL(state, Attr::Conceal, true);

    // Conceal off
    push(vt, "\e[28m");
    ASSERT_PEN_BOOL(state, Attr::Conceal, false);
}

// ============================================================================
// SGR 9/29 -- Strikethrough on/off
// ============================================================================

// SGR 9 enables strikethrough, SGR 29 disables it
TEST(seq_sgr_strikethrough_on_off)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Strikethrough on
    push(vt, "\e[9m");
    ASSERT_PEN_BOOL(state, Attr::Strike, true);

    // Strikethrough off
    push(vt, "\e[29m");
    ASSERT_PEN_BOOL(state, Attr::Strike, false);
}

// ============================================================================
// SGR 10-11 -- Font selection
// ============================================================================

// SGR 11 selects alternate font 1, SGR 10 returns to default font 0
TEST(seq_sgr_font_select)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    callbacks_clear();

    // Select alternate font 1
    push(vt, "\e[11m");
    ASSERT_PEN_INT(state, Attr::Font, 1);

    // Return to default font 0
    push(vt, "\e[10m");
    ASSERT_PEN_INT(state, Attr::Font, 0);
}
