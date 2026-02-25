// test_29_state_fallback.cpp — state fallback tests
// Ported from upstream libvterm t/29state_fallback.test

#include "harness.h"

// Unrecognised control
TEST(state_fallback_control)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs_no_scrollrect);
    state.set_fallbacks(fallback_cbs);
    state.reset(true);
    fallback_clear();

    push(vt, "\x03");
    ASSERT_EQ(g_fallback.control_count, 1);
    ASSERT_EQ(g_fallback.control[0].control, 0x03);
}

// Unrecognised CSI
TEST(state_fallback_csi)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs_no_scrollrect);
    state.set_fallbacks(fallback_cbs);
    state.reset(true);
    fallback_clear();

    // \e[?15;2z — CSI with leader '?' (0x3f), command 'z' (0x7a), args 15,2
    push(vt, "\x1b" "[?15;2z");
    ASSERT_EQ(g_fallback.csi_count, 1);
    ASSERT_EQ(g_fallback.csi[0].command, 'z');
    ASSERT_EQ(g_fallback.csi[0].leader[0], '?');
    ASSERT_EQ(g_fallback.csi[0].args[0], 15);
    ASSERT_EQ(g_fallback.csi[0].args[1], 2);
}

// Unrecognised OSC
TEST(state_fallback_osc)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs_no_scrollrect);
    state.set_fallbacks(fallback_cbs);
    state.reset(true);
    fallback_clear();

    // \e]27;Something\e\\ — OSC command 27, data "Something", ST terminated
    push(vt, "\x1b" "]27;Something\x1b" "\\");
    ASSERT_EQ(g_fallback.osc_count, 1);
    ASSERT_EQ(g_fallback.osc[0].command, 27);
    ASSERT_EQ(g_fallback.osc[0].datalen, 9);
    ASSERT_TRUE(std::string_view(g_fallback.osc[0].data.data(), 9) == "Something");
}

// Unrecognised DCS
TEST(state_fallback_dcs)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs_no_scrollrect);
    state.set_fallbacks(fallback_cbs);
    state.reset(true);
    fallback_clear();

    // \ePz123\e\\ — DCS with command byte 'z' and data "123", ST terminated
    // The DCS callback stores command bytes first then fragment data: "z123"
    push(vt, "\x1b" "Pz123\x1b" "\\");
    ASSERT_EQ(g_fallback.dcs_count, 1);
    ASSERT_EQ(g_fallback.dcs[0].datalen, 4);
    ASSERT_TRUE(std::string_view(g_fallback.dcs[0].data.data(), 4) == "z123");
}

// Unrecognised APC
TEST(state_fallback_apc)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs_no_scrollrect);
    state.set_fallbacks(fallback_cbs);
    state.reset(true);
    fallback_clear();

    // \e_z123\e\\ — APC with data "z123", ST terminated
    push(vt, "\x1b" "_z123\x1b" "\\");
    ASSERT_EQ(g_fallback.apc_count, 1);
    ASSERT_EQ(g_fallback.apc[0].datalen, 4);
    ASSERT_TRUE(std::string_view(g_fallback.apc[0].data.data(), 4) == "z123");
}

// Unrecognised PM
TEST(state_fallback_pm)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs_no_scrollrect);
    state.set_fallbacks(fallback_cbs);
    state.reset(true);
    fallback_clear();

    // \e^z123\e\\ — PM with data "z123", ST terminated
    push(vt, "\x1b" "^z123\x1b" "\\");
    ASSERT_EQ(g_fallback.pm_count, 1);
    ASSERT_EQ(g_fallback.pm[0].datalen, 4);
    ASSERT_TRUE(std::string_view(g_fallback.pm[0].data.data(), 4) == "z123");
}

// Unrecognised SOS
TEST(state_fallback_sos)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs_no_scrollrect);
    state.set_fallbacks(fallback_cbs);
    state.reset(true);
    fallback_clear();

    // \eXz123\e\\ — SOS with data "z123", ST terminated
    push(vt, "\x1b" "Xz123\x1b" "\\");
    ASSERT_EQ(g_fallback.sos_count, 1);
    ASSERT_EQ(g_fallback.sos[0].datalen, 4);
    ASSERT_TRUE(std::string_view(g_fallback.sos[0].data.data(), 4) == "z123");
}
