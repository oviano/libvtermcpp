// test_seq_xtversion.cpp -- per-sequence tests for XTVERSION (CSI > q)
//
// XTVERSION queries the terminal version string.
// Response: DCS > | <version-string> ST
//
// libvterm responds with: DCS > | libvterm(0.3) ST

#include "harness.h"

// ============================================================================
// XTVERSION query -- full response
// ============================================================================

// CSI > q should produce \eP>|libvterm(0.3)\e\\ (19 bytes)
TEST(seq_xtversion_query)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    output_init(vt);
    state.reset(true);
    callbacks_clear();
    output_clear();

    push(vt, "\e[>q");
    ASSERT_OUTPUT_BYTES("\eP>|libvterm(0.3)\e\\", 19);
}

// ============================================================================
// XTVERSION response format validation
// ============================================================================

// Verify the DCS response starts with \eP>| and ends with \e\\ (ST)
TEST(seq_xtversion_format)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    output_init(vt);
    state.reset(true);
    callbacks_clear();
    output_clear();

    push(vt, "\e[>q");

    // Must have at least the DCS prefix (4 bytes) + ST (2 bytes)
    ASSERT_TRUE(g_output_len >= 6);

    // Starts with DCS >| : \e P > |
    ASSERT_EQ(g_output_buf[0], '\e');
    ASSERT_EQ(g_output_buf[1], 'P');
    ASSERT_EQ(g_output_buf[2], '>');
    ASSERT_EQ(g_output_buf[3], '|');

    // Ends with ST: \e \\ (backslash)
    ASSERT_EQ(g_output_buf[g_output_len - 2], '\e');
    ASSERT_EQ(g_output_buf[g_output_len - 1], '\\');
}
