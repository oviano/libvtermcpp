// test_02_parser.cpp — parser layer tests
// Ported from upstream libvterm t/02parser.test

#include "harness.h"

// Basic text
TEST(parser_basic_text)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    vt.parser_set_callbacks(parser_cbs);
    parser_clear();

    push(vt, "hello");
    ASSERT_EQ(g_parser.text_count, 1);
    ASSERT_EQ(g_parser.text[0].len, 5);
    ASSERT_EQ(g_parser.text[0].bytes[0], 'h');
    ASSERT_EQ(g_parser.text[0].bytes[4], 'o');
}

// C0 control
TEST(parser_c0_control)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    vt.parser_set_callbacks(parser_cbs);
    parser_clear();

    push(vt, {"\x03", 1});
    ASSERT_EQ(g_parser.control_count, 1);
    ASSERT_EQ(g_parser.control[0].control, 0x03);
}

// Mixed text and C0
TEST(parser_mixed_text_c0)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    vt.parser_set_callbacks(parser_cbs);
    parser_clear();

    push(vt, {"AB\x03" "CD", 5});
    ASSERT_EQ(g_parser.text_count, 2);
    ASSERT_EQ(g_parser.text[0].len, 2);
    ASSERT_EQ(g_parser.control_count, 1);
    ASSERT_EQ(g_parser.control[0].control, 0x03);
    ASSERT_EQ(g_parser.text[1].len, 2);
}

// ESC sequence
TEST(parser_escape)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    vt.parser_set_callbacks(parser_cbs);
    parser_clear();

    push(vt, "\e=");
    ASSERT_EQ(g_parser.escape_count, 1);
    ASSERT_STR_EQ(g_parser.escape[0].seq.data(), "=");
}

// CSI with no args
TEST(parser_csi_no_args)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    vt.parser_set_callbacks(parser_cbs);
    parser_clear();

    push(vt, "\e[a");
    ASSERT_EQ(g_parser.csi_count, 1);
    ASSERT_EQ(g_parser.csi[0].command, 'a');
    ASSERT_EQ(g_parser.csi[0].argcount, 1); // one missing arg
}

// CSI with one arg
TEST(parser_csi_one_arg)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    vt.parser_set_callbacks(parser_cbs);
    parser_clear();

    push(vt, "\e[9b");
    ASSERT_EQ(g_parser.csi_count, 1);
    ASSERT_EQ(g_parser.csi[0].command, 'b');
    ASSERT_EQ(g_parser.csi[0].argcount, 1);
    ASSERT_EQ(g_parser.csi[0].args[0], 9);
}

// CSI with two args
TEST(parser_csi_two_args)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    vt.parser_set_callbacks(parser_cbs);
    parser_clear();

    push(vt, "\e[3;4c");
    ASSERT_EQ(g_parser.csi_count, 1);
    ASSERT_EQ(g_parser.csi[0].command, 'c');
    ASSERT_EQ(g_parser.csi[0].argcount, 2);
    ASSERT_EQ(g_parser.csi[0].args[0], 3);
    ASSERT_EQ(g_parser.csi[0].args[1], 4);
}

// CSI with leader
TEST(parser_csi_leader)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    vt.parser_set_callbacks(parser_cbs);
    parser_clear();

    push(vt, "\e[?5c");
    ASSERT_EQ(g_parser.csi_count, 1);
    ASSERT_EQ(g_parser.csi[0].command, 'c');
    ASSERT_STR_EQ(g_parser.csi[0].leader.data(), "?");
    ASSERT_EQ(g_parser.csi[0].args[0], 5);
}

// CSI with intermediate
TEST(parser_csi_intermed)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    vt.parser_set_callbacks(parser_cbs);
    parser_clear();

    push(vt, "\e[5 q");
    ASSERT_EQ(g_parser.csi_count, 1);
    ASSERT_EQ(g_parser.csi[0].command, 'q');
    ASSERT_STR_EQ(g_parser.csi[0].intermed.data(), " ");
    ASSERT_EQ(g_parser.csi[0].args[0], 5);
}

// OSC BEL terminated
TEST(parser_osc_bel)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    vt.parser_set_callbacks(parser_cbs);
    parser_clear();

    push(vt, "\e]1;Hello\x07");
    ASSERT_EQ(g_parser.osc_count, 1);
    ASSERT_EQ(g_parser.osc[0].command, 1);
    ASSERT_EQ(g_parser.osc[0].final_, true);
    ASSERT_EQ(g_parser.osc[0].datalen, 5);
    ASSERT_TRUE(std::string_view(g_parser.osc[0].data.data(), 5) == "Hello");
}

// OSC ST terminated
TEST(parser_osc_st)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    vt.parser_set_callbacks(parser_cbs);
    parser_clear();

    push(vt, "\e]1;Hello\e\\");
    ASSERT_EQ(g_parser.osc_count, 1);
    ASSERT_EQ(g_parser.osc[0].command, 1);
    ASSERT_EQ(g_parser.osc[0].final_, true);
}

// NUL ignored by default
TEST(parser_nul_ignored)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    vt.parser_set_callbacks(parser_cbs);
    parser_clear();

    push(vt, {"\x00", 1});
    ASSERT_EQ(g_parser.control_count, 0);
}

// CAN cancels CSI
TEST(parser_can_cancels_csi)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    vt.parser_set_callbacks(parser_cbs);
    parser_clear();

    push(vt, "\e[12\x18""3");
    // CAN cancels the CSI, then '3' is text
    ASSERT_EQ(g_parser.csi_count, 0);
    ASSERT_EQ(g_parser.text_count, 1);
    ASSERT_EQ(g_parser.text[0].bytes[0], '3');
}

// Split write mid-CSI
TEST(parser_split_csi)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    vt.parser_set_callbacks(parser_cbs);
    parser_clear();

    push(vt, "\e[12");
    ASSERT_EQ(g_parser.csi_count, 0);
    push(vt, ";34H");
    ASSERT_EQ(g_parser.csi_count, 1);
    ASSERT_EQ(g_parser.csi[0].command, 'H');
    ASSERT_EQ(g_parser.csi[0].argcount, 2);
    ASSERT_EQ(g_parser.csi[0].args[0], 12);
    ASSERT_EQ(g_parser.csi[0].args[1], 34);
}

// C1 controls in non-UTF8 mode
TEST(parser_c1_8bit)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    vt.parser_set_callbacks(parser_cbs);
    parser_clear();

    // 0x9b = CSI in 8-bit mode
    push(vt, {"\x9b""3;4H", 5});
    ASSERT_EQ(g_parser.csi_count, 1);
    ASSERT_EQ(g_parser.csi[0].command, 'H');
    ASSERT_EQ(g_parser.csi[0].args[0], 3);
    ASSERT_EQ(g_parser.csi[0].args[1], 4);
}
