// test_40_state_selection.cpp — state selection (OSC 52 clipboard) tests
// Ported from upstream libvterm t/40state_selection.test

#include "harness.h"

// ============================================================================
// Selection callback recording
// ============================================================================

static constexpr int32_t sel_log_max  = 16;
static constexpr int32_t sel_data_max = 256;

struct selection_set_record {
    SelectionMask mask;
    std::array<char, sel_data_max> data{};
    size_t datalen;
    bool initial;
    bool final_;
};

struct SelectionLog {
    std::array<selection_set_record, sel_log_max> set{};
    int32_t set_count;

    SelectionMask query_mask;
    int32_t query_count;
};

static SelectionLog g_sel;

static void sel_clear()
{
    g_sel = {};
}

struct TestSelectionCallbacks : SelectionCallbacks {
    bool on_set(SelectionMask mask, StringFragment frag) override {
        if(g_sel.set_count < sel_log_max) {
            auto& r = g_sel.set[g_sel.set_count];
            r.mask = mask;
            if(!frag.str.empty()) {
                size_t to_copy = std::min(frag.str.size(), static_cast<size_t>(sel_data_max));
                std::copy_n(frag.str.data(), to_copy, r.data.data());
                r.datalen = to_copy;
            } else {
                r.datalen = 0;
            }
            r.initial = frag.initial;
            r.final_ = frag.final_;
        }
        g_sel.set_count++;
        return true;
    }
    bool on_query(SelectionMask mask) override {
        g_sel.query_mask = mask;
        g_sel.query_count++;
        return true;
    }
};

static TestSelectionCallbacks selection_cbs_local;

// Helper to assert selection-set data matches expected bytes
#define ASSERT_SEL_SET(idx, exp_mask, exp_initial, exp_final, exp_data, exp_len) \
    do {                                                                    \
        ASSERT_TRUE((idx) < g_sel.set_count);                               \
        ASSERT_EQ(g_sel.set[(idx)].mask, (exp_mask));                       \
        ASSERT_EQ(g_sel.set[(idx)].initial, (exp_initial));                 \
        ASSERT_EQ(g_sel.set[(idx)].final_, (exp_final));                     \
        ASSERT_EQ(g_sel.set[(idx)].datalen, static_cast<size_t>(exp_len));  \
        if ((exp_len) > 0) {                                                \
            if (std::string_view(g_sel.set[(idx)].data.data(), (exp_len)) != std::string_view((exp_data), (exp_len))) { \
                std::cerr << std::format("  FAIL {}:{}: sel set[{}] data mismatch\n", \
                        __FILE__, __LINE__, (idx));                          \
                (*_test_failures)++;                                         \
                return;                                                     \
            }                                                               \
        }                                                                   \
    } while (0)

// Set clipboard; final chunk len 4
TEST(state_selection_set_final_len4)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.set_selection_callbacks(selection_cbs_local, 16);
    state.reset(true);

    sel_clear();
    // SGVsbG8s decodes to "Hello," (6 bytes, base64 len 8, 8/4=2 full quads)
    push(vt, "\e]52;c;SGVsbG8s\e\\");
    // selection-set mask=0001 ["Hello,"]
    ASSERT_EQ(g_sel.set_count, 1);
    ASSERT_SEL_SET(0, SelectionMask::Clipboard, true, true, "Hello,", 6);
}

// Set clipboard; final chunk len 3
TEST(state_selection_set_final_len3)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.set_selection_callbacks(selection_cbs_local, 16);
    state.reset(true);

    sel_clear();
    // SGVsbG8sIHc= decodes to "Hello, w" (8 bytes)
    push(vt, "\e]52;c;SGVsbG8sIHc=\e\\");
    // selection-set mask=0001 ["Hello, w"]
    ASSERT_EQ(g_sel.set_count, 1);
    ASSERT_SEL_SET(0, SelectionMask::Clipboard, true, true, "Hello, w", 8);
}

// Set clipboard; final chunk len 2
TEST(state_selection_set_final_len2)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.set_selection_callbacks(selection_cbs_local, 16);
    state.reset(true);

    sel_clear();
    // SGVsbG8sIHdvcmxkCg== decodes to "Hello, world\n" (13 bytes)
    push(vt, "\e]52;c;SGVsbG8sIHdvcmxkCg==\e\\");
    // selection-set mask=0001 ["Hello, world\n"]
    ASSERT_EQ(g_sel.set_count, 1);
    ASSERT_SEL_SET(0, SelectionMask::Clipboard, true, true, "Hello, world\n", 13);
}

// Set clipboard; split between chunks
//
// The OSC 52 data "SGVsbG8s" is split across two PUSH calls at a base64
// quad boundary: "SGVs" | "bG8s".  The selection buffer is only 16 bytes so
// the decoded data is delivered in two fragments.
//
// First PUSH delivers the initial fragment with "Hel" (3 bytes decoded from
// the first quad "SGVs").  Second PUSH delivers the final fragment with "lo,"
// (3 bytes decoded from the second quad "bG8s").
TEST(state_selection_set_split_between_chunks)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.set_selection_callbacks(selection_cbs_local, 16);
    state.reset(true);

    sel_clear();
    push(vt, "\e]52;c;SGVs");
    // selection-set mask=0001 ["Hel"
    ASSERT_EQ(g_sel.set_count, 1);
    ASSERT_SEL_SET(0, SelectionMask::Clipboard, true, false, "Hel", 3);

    push(vt, "bG8s\e\\");
    // selection-set mask=0001 "lo,"]
    ASSERT_EQ(g_sel.set_count, 2);
    ASSERT_SEL_SET(1, SelectionMask::Clipboard, false, true, "lo,", 3);
}

// Set clipboard; split within chunk
//
// The base64 data is split mid-quad: "SGVsbG" | "8s".  The first PUSH
// decodes the first full quad "SGVs" -> "Hel" but the leftover "bG" is
// held as partial state.  The second PUSH completes the quad "bG8s" -> "lo,"
// and delivers it as the final fragment.
TEST(state_selection_set_split_within_chunk)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.set_selection_callbacks(selection_cbs_local, 16);
    state.reset(true);

    sel_clear();
    push(vt, "\e]52;c;SGVsbG");
    // selection-set mask=0001 ["Hel"
    ASSERT_EQ(g_sel.set_count, 1);
    ASSERT_SEL_SET(0, SelectionMask::Clipboard, true, false, "Hel", 3);

    push(vt, "8s\e\\");
    // selection-set mask=0001 "lo,"]
    ASSERT_EQ(g_sel.set_count, 2);
    ASSERT_SEL_SET(1, SelectionMask::Clipboard, false, true, "lo,", 3);
}

// Set clipboard; empty first chunk
//
// First PUSH sends just the OSC header "\e]52;c;" with no base64 data.
// No callback is fired yet.  The second PUSH sends the full base64 payload
// and the ST terminator, delivering the complete data in one fragment.
TEST(state_selection_set_empty_first_chunk)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.set_selection_callbacks(selection_cbs_local, 16);
    state.reset(true);

    sel_clear();
    push(vt, "\e]52;c;");
    ASSERT_EQ(g_sel.set_count, 0);

    push(vt, "SGVsbG8s\e\\");
    // selection-set mask=0001 ["Hello,"]
    ASSERT_EQ(g_sel.set_count, 1);
    ASSERT_SEL_SET(0, SelectionMask::Clipboard, true, true, "Hello,", 6);
}

// Set clipboard; empty final chunk
//
// First PUSH sends the header and base64 data but no ST terminator.
// This delivers the initial fragment with decoded "Hello," but without
// the final flag.  The second PUSH sends just the ST terminator, which
// triggers a final (empty) fragment.
TEST(state_selection_set_empty_final_chunk)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.set_selection_callbacks(selection_cbs_local, 16);
    state.reset(true);

    sel_clear();
    push(vt, "\e]52;c;SGVsbG8s");
    // selection-set mask=0001 ["Hello,"
    ASSERT_EQ(g_sel.set_count, 1);
    ASSERT_SEL_SET(0, SelectionMask::Clipboard, true, false, "Hello,", 6);

    push(vt, "\e\\");
    // selection-set mask=0001 ] (empty final fragment)
    ASSERT_EQ(g_sel.set_count, 2);
    ASSERT_SEL_SET(1, SelectionMask::Clipboard, false, true, "", 0);
}

// Set clipboard; longer than buffer
//
// "LS0t" repeated 10 times is 40 base64 chars, decoding to "---" x 10 = 30
// dashes.  With a 16-byte selection buffer, the data is delivered in two
// fragments of 15 bytes each.
TEST(state_selection_set_longer_than_buffer)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.set_selection_callbacks(selection_cbs_local, 16);
    state.reset(true);

    sel_clear();
    // "LS0t" x 10 = "LS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0t"
    push(vt, "\e]52;c;LS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0t\e\\");
        // selection-set mask=0001 ["-"x15  (initial, not final)
    // selection-set mask=0001  "-"x15] (not initial, final)
    ASSERT_EQ(g_sel.set_count, 2);
    ASSERT_SEL_SET(0, SelectionMask::Clipboard, true, false,
                   "---------------", 15);
    ASSERT_SEL_SET(1, SelectionMask::Clipboard, false, true,
                   "---------------", 15);
}

// Clear clipboard
TEST(state_selection_clear)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.set_selection_callbacks(selection_cbs_local, 16);
    state.reset(true);

    sel_clear();
    push(vt, "\e]52;c;\e\\");
    // selection-set mask=0001 [] (initial+final, empty)
    ASSERT_EQ(g_sel.set_count, 1);
    ASSERT_SEL_SET(0, SelectionMask::Clipboard, true, true, "", 0);
}

// Set invalid data clears and ignores
TEST(state_selection_set_invalid_data)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.set_selection_callbacks(selection_cbs_local, 16);
    state.reset(true);

    sel_clear();
    // The '*' is invalid base64, so the selection is cleared
    push(vt, "\e]52;c;SGVs*SGVsbG8s\e\\");
    // selection-set mask=0001 [] (initial+final, empty -- invalid data clears)
    ASSERT_EQ(g_sel.set_count, 1);
    ASSERT_SEL_SET(0, SelectionMask::Clipboard, true, true, "", 0);
}

// Query clipboard
TEST(state_selection_query)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.set_selection_callbacks(selection_cbs_local, 16);
    state.reset(true);

    sel_clear();
    push(vt, "\e]52;c;?\e\\");
    // selection-query mask=0001
    ASSERT_EQ(g_sel.query_count, 1);
    ASSERT_EQ(g_sel.query_mask, SelectionMask::Clipboard);
}

// Send clipboard; final chunk len 4
//
// Sending "Hello," (6 bytes) via vterm_state_send_selection produces:
//   "\e]52;c;" + "SGVsbG8s" + "\e\\"
//
// 6 bytes -> 2 full base64 quads (8 chars), no padding needed.
TEST(state_selection_send_final_len4)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.set_selection_callbacks(selection_cbs_local, 16);
    state.reset(true);
    output_init(vt);

    output_clear();
    StringFragment frag;
    frag.str = "Hello,";
    frag.initial = true;
    frag.final_ = true;
    state.send_selection(SelectionMask::Clipboard, frag);

    // output "\e]52;c;" + "SGVsbG8s" + "\e\\"
    ASSERT_OUTPUT_BYTES("\e]52;c;" "SGVsbG8s" "\e\\", 7 + 8 + 2);
}

// Send clipboard; final chunk len 3
//
// Sending "Hello, w" (8 bytes) produces:
//   "\e]52;c;" + "SGVsbG8s" + "IHc=\e\\"
//
// 8 bytes -> 2 full quads (8 chars) + 1 partial (2 bytes -> 4 chars with padding)
// The buffer is 16 bytes, so after encoding 6 bytes (2 quads = 8 b64 chars)
// the buffer is flushed, then the remaining 2 bytes produce "IHc=" on final.
TEST(state_selection_send_final_len3)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.set_selection_callbacks(selection_cbs_local, 16);
    state.reset(true);
    output_init(vt);

    output_clear();
    StringFragment frag;
    frag.str = "Hello, w";
    frag.initial = true;
    frag.final_ = true;
    state.send_selection(SelectionMask::Clipboard, frag);

    // output "\e]52;c;" + "SGVsbG8s" + "IHc=\e\\"
    ASSERT_OUTPUT_BYTES("\e]52;c;" "SGVsbG8s" "IHc=" "\e\\", 7 + 8 + 4 + 2);
}

// Send clipboard; final chunk len 2
//
// Sending "Hello, world\n" (13 bytes) produces:
//   "\e]52;c;" + "SGVsbG8sIHdvcmxk" + "Cg==\e\\"
//
// 13 bytes -> 4 full quads (12 bytes -> 16 b64 chars) + 1 byte padded to "Cg=="
TEST(state_selection_send_final_len2)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.set_selection_callbacks(selection_cbs_local, 16);
    state.reset(true);
    output_init(vt);

    output_clear();
    StringFragment frag;
    frag.str = std::string_view{"Hello, world\n", 13};
    frag.initial = true;
    frag.final_ = true;
    state.send_selection(SelectionMask::Clipboard, frag);

    // output "\e]52;c;" + "SGVsbG8sIHdvcmxk" + "Cg==\e\\"
    ASSERT_OUTPUT_BYTES("\e]52;c;" "SGVsbG8sIHdvcmxk" "Cg==" "\e\\", 7 + 16 + 4 + 2);
}

// Send clipboard; split between chunks
//
// Two separate vterm_state_send_selection calls:
//   1. initial frag "Hel" -> outputs "\e]52;c;" + "SGVs"
//   2. final frag "lo,"   -> outputs "bG8s" + "\e\\"
TEST(state_selection_send_split_between_chunks)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.set_selection_callbacks(selection_cbs_local, 16);
    state.reset(true);
    output_init(vt);

    // First fragment: initial, not final
    output_clear();
    StringFragment frag1;
    frag1.str = "Hel";
    frag1.initial = true;
    frag1.final_ = false;
    state.send_selection(SelectionMask::Clipboard, frag1);

    // output "\e]52;c;" + "SGVs"
    ASSERT_OUTPUT_BYTES("\e]52;c;" "SGVs", 7 + 4);

    // Second fragment: not initial, final
    output_clear();
    StringFragment frag2;
    frag2.str = "lo,";
    frag2.initial = false;
    frag2.final_ = true;
    state.send_selection(SelectionMask::Clipboard, frag2);

    // output "bG8s" + "\e\\"
    ASSERT_OUTPUT_BYTES("bG8s" "\e\\", 4 + 2);
}

// Send clipboard; split within chunk
//
// Two separate vterm_state_send_selection calls where the split is not
// on a 3-byte boundary:
//   1. initial frag "Hello" (5 bytes) -> outputs "\e]52;c;" + "SGVs"
//      (encodes first 3 bytes "Hel" -> "SGVs", holds "lo" as partial)
//   2. final frag ","  (1 byte) -> outputs "bG8s" + "\e\\"
//      (combines "lo" + "," -> "lo," -> "bG8s")
TEST(state_selection_send_split_within_chunk)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.set_selection_callbacks(selection_cbs_local, 16);
    state.reset(true);
    output_init(vt);

    // First fragment: initial, not final
    output_clear();
    StringFragment frag1;
    frag1.str = "Hello";
    frag1.initial = true;
    frag1.final_ = false;
    state.send_selection(SelectionMask::Clipboard, frag1);

    // output "\e]52;c;" + "SGVs"
    ASSERT_OUTPUT_BYTES("\e]52;c;" "SGVs", 7 + 4);

    // Second fragment: not initial, final
    output_clear();
    StringFragment frag2;
    frag2.str = ",";
    frag2.initial = false;
    frag2.final_ = true;
    state.send_selection(SelectionMask::Clipboard, frag2);

    // output "bG8s" + "\e\\"
    ASSERT_OUTPUT_BYTES("bG8s" "\e\\", 4 + 2);
}
