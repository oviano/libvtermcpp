/*
 * harness.h — test helpers for libvtermcpp tests
 *
 * Provides:
 *   - push() wrapper for Terminal::write
 *   - Output capture (keyboard/mouse output)
 *   - Callback recording for state and screen layers
 *   - Assertion macros for cursor, screen rows, pen attributes, etc.
 */

#ifndef HARNESS_H
#define HARNESS_H

#include "test.h"
#include "vterm/vterm.h"
#include "../src/internal.h"

#include <algorithm>
#include <array>
#include <span>
#include <string_view>

using namespace vterm;

// ============================================================================
// Push helper
// ============================================================================

inline void push(Terminal& vt, std::string_view str) {
    (void)vt.write(std::span<const char>(str.data(), str.size()));
}


// ============================================================================
// Output capture
// ============================================================================

inline constexpr int32_t OUTPUT_BUF_MAX = 4096;

inline std::array<char, OUTPUT_BUF_MAX> g_output_buf;
inline size_t g_output_len = 0;

inline void output_init(Terminal& vt) {
    g_output_len = 0;
    vt.set_output_callback([](std::span<const char> data) {
        if(g_output_len + data.size() <= OUTPUT_BUF_MAX)
            std::copy_n(data.data(), data.size(), g_output_buf.data() + g_output_len);
        g_output_len += data.size();
    });
}

inline void output_clear() {
    g_output_len = 0;
}

#define ASSERT_OUTPUT_BYTES(expected, expected_len)                          \
    do {                                                                    \
        size_t _elen = (expected_len);                                      \
        if (g_output_len != _elen || !std::equal(g_output_buf.data(), g_output_buf.data() + _elen, expected)) { \
            std::cerr << std::format("  FAIL {}:{}: output mismatch (got {} bytes, expected {})\n", \
                    __FILE__, __LINE__, g_output_len, _elen);               \
            (*_test_failures)++;                                             \
            return;                                                         \
        }                                                                   \
        output_clear();                                                     \
    } while (0)

// ============================================================================
// Cursor assertion
// ============================================================================

#define ASSERT_CURSOR(st, expected_row, expected_col)                       \
    do {                                                                    \
        Pos _pos = (st).cursor_pos();                                       \
        if (_pos.row != (expected_row) || _pos.col != (expected_col)) {     \
            std::cerr << std::format("  FAIL {}:{}: cursor at ({},{}), expected ({},{})\n", \
                    __FILE__, __LINE__, _pos.row, _pos.col,                 \
                    (expected_row), (expected_col));                         \
            (*_test_failures)++;                                             \
            return;                                                         \
        }                                                                   \
    } while (0)

// ============================================================================
// Screen row assertion
// ============================================================================

#define ASSERT_SCREEN_ROW(vt, scr, row, expected)                          \
    do {                                                                    \
        int32_t _cols = (vt).cols();                                        \
        std::array<char, 1024> _rowbuf{};                                   \
        Rect _rect{};                                                       \
        _rect.start_row = (row); _rect.end_row = (row)+1;                  \
        _rect.start_col = 0; _rect.end_col = _cols;                        \
        size_t _len = (scr).get_text(std::span{_rowbuf.data(), _rowbuf.size()-1}, _rect); \
        _rowbuf[_len] = '\0';                                               \
        while (_len > 0 && _rowbuf[_len-1] == ' ') _rowbuf[--_len] = '\0'; \
        if (std::string_view(_rowbuf.data(), _len) != std::string_view(expected)) { \
            std::cerr << std::format("  FAIL {}:{}: screen_row {} = \"{}\", expected \"{}\"\n", \
                    __FILE__, __LINE__, (row), _rowbuf.data(), (expected));  \
            (*_test_failures)++;                                             \
            return;                                                         \
        }                                                                   \
    } while (0)

// ============================================================================
// Pen assertions
// ============================================================================

#define ASSERT_PEN_BOOL(st, attr, expected)                                \
    do {                                                                    \
        Value _val{};                                                       \
        (void)(st).get_penattr(attr, _val);                                 \
        if (_val.boolean != (expected)) {                                   \
            std::cerr << std::format("  FAIL {}:{}: pen attr {} = {}, expected {}\n", \
                    __FILE__, __LINE__, static_cast<int32_t>(attr), _val.boolean, \
                    static_cast<int32_t>(expected));                         \
            (*_test_failures)++;                                             \
            return;                                                         \
        }                                                                   \
    } while (0)

#define ASSERT_PEN_INT(st, attr, expected)                                 \
    do {                                                                    \
        Value _val{};                                                       \
        (void)(st).get_penattr(attr, _val);                                 \
        if (_val.number != (expected)) {                                    \
            std::cerr << std::format("  FAIL {}:{}: pen attr {} = {}, expected {}\n", \
                    __FILE__, __LINE__, static_cast<int32_t>(attr), _val.number, \
                    static_cast<int32_t>(expected));                         \
            (*_test_failures)++;                                             \
            return;                                                         \
        }                                                                   \
    } while (0)

#define ASSERT_PEN_COLOR_RGB(st, attr, er, eg, eb)                         \
    do {                                                                    \
        Value _val{};                                                       \
        (void)(st).get_penattr(attr, _val);                                 \
        Color _c = _val.color;                                              \
        (st).convert_color_to_rgb(_c);                                      \
        if (_c.rgb.red != (er) || _c.rgb.green != (eg) || _c.rgb.blue != (eb)) { \
            std::cerr << std::format("  FAIL {}:{}: pen color = rgb({},{},{}), expected rgb({},{},{})\n", \
                    __FILE__, __LINE__, _c.rgb.red, _c.rgb.green, _c.rgb.blue, \
                    (er), (eg), (eb));                                       \
            (*_test_failures)++;                                             \
            return;                                                         \
        }                                                                   \
    } while (0)

#define ASSERT_PEN_COLOR_IDX(st, attr, expected_idx)                       \
    do {                                                                    \
        Value _val{};                                                       \
        (void)(st).get_penattr(attr, _val);                                 \
        if (!_val.color.is_indexed() || _val.color.indexed.idx != (expected_idx)) { \
            std::cerr << std::format("  FAIL {}:{}: pen color index = {}, expected {}\n", \
                    __FILE__, __LINE__, _val.color.indexed.idx, (expected_idx)); \
            (*_test_failures)++;                                             \
            return;                                                         \
        }                                                                   \
    } while (0)

#define ASSERT_PEN_COLOR_DEFAULT_FG(st, attr, er, eg, eb)                  \
    do {                                                                    \
        Value _val{};                                                       \
        (void)(st).get_penattr(attr, _val);                                 \
        Color _c = _val.color;                                              \
        if (!_c.is_default_fg()) {                                          \
            std::cerr << std::format("  FAIL {}:{}: pen color not default fg\n", \
                    __FILE__, __LINE__);                                     \
            (*_test_failures)++;                                             \
            return;                                                         \
        }                                                                   \
        (st).convert_color_to_rgb(_c);                                      \
        if (_c.rgb.red != (er) || _c.rgb.green != (eg) || _c.rgb.blue != (eb)) { \
            std::cerr << std::format("  FAIL {}:{}: pen color = rgb({},{},{}), expected rgb({},{},{})\n", \
                    __FILE__, __LINE__, _c.rgb.red, _c.rgb.green, _c.rgb.blue, \
                    (er), (eg), (eb));                                       \
            (*_test_failures)++;                                             \
            return;                                                         \
        }                                                                   \
    } while (0)

#define ASSERT_PEN_COLOR_DEFAULT_BG(st, attr, er, eg, eb)                  \
    do {                                                                    \
        Value _val{};                                                       \
        (void)(st).get_penattr(attr, _val);                                 \
        Color _c = _val.color;                                              \
        if (!_c.is_default_bg()) {                                          \
            std::cerr << std::format("  FAIL {}:{}: pen color not default bg\n", \
                    __FILE__, __LINE__);                                     \
            (*_test_failures)++;                                             \
            return;                                                         \
        }                                                                   \
        (st).convert_color_to_rgb(_c);                                      \
        if (_c.rgb.red != (er) || _c.rgb.green != (eg) || _c.rgb.blue != (eb)) { \
            std::cerr << std::format("  FAIL {}:{}: pen color = rgb({},{},{}), expected rgb({},{},{})\n", \
                    __FILE__, __LINE__, _c.rgb.red, _c.rgb.green, _c.rgb.blue, \
                    (er), (eg), (eb));                                       \
            (*_test_failures)++;                                             \
            return;                                                         \
        }                                                                   \
    } while (0)

// ============================================================================
// Line info assertion
// ============================================================================

#define ASSERT_LINEINFO(st, row, field, expected)                          \
    do {                                                                    \
        const LineInfo& _li = (st).get_lineinfo(row);                       \
        if (_li.field != (expected)) {                                      \
            std::cerr << std::format("  FAIL {}:{}: lineinfo[{}].{} = {}, expected {}\n", \
                    __FILE__, __LINE__, (row), #field,                       \
                    static_cast<int32_t>(_li.field), static_cast<int32_t>(expected)); \
            (*_test_failures)++;                                             \
            return;                                                         \
        }                                                                   \
    } while (0)

// ============================================================================
// Screen cell assertion
// ============================================================================

#define ASSERT_SCREEN_CELL_CHAR(scr, row, col, expected_char)              \
    do {                                                                    \
        Pos _p{(row), (col)};                                               \
        ScreenCell _cell{};                                                 \
        (scr).get_cell(_p, _cell);                                          \
        if (_cell.chars[0] != static_cast<uint32_t>(expected_char)) {                  \
            std::cerr << std::format("  FAIL {}:{}: cell({},{}) char = 0x{:x}, expected 0x{:x}\n", \
                    __FILE__, __LINE__, (row), (col),                        \
                    _cell.chars[0], static_cast<uint32_t>(expected_char));   \
            (*_test_failures)++;                                             \
            return;                                                         \
        }                                                                   \
    } while (0)

#define ASSERT_SCREEN_CELL_WIDTH(scr, row, col, expected_width)            \
    do {                                                                    \
        Pos _p{(row), (col)};                                               \
        ScreenCell _cell{};                                                 \
        (scr).get_cell(_p, _cell);                                          \
        if (_cell.width != (expected_width)) {                              \
            std::cerr << std::format("  FAIL {}:{}: cell({},{}) width = {}, expected {}\n", \
                    __FILE__, __LINE__, (row), (col),                        \
                    _cell.width, (expected_width));                          \
            (*_test_failures)++;                                             \
            return;                                                         \
        }                                                                   \
    } while (0)

// ============================================================================
// State callback recording
// ============================================================================

inline constexpr int32_t CALLBACK_LOG_MAX = 128;

struct putglyph_record {
    std::array<uint32_t, 6> chars{};
    int32_t width = 0;
    int32_t row = 0, col = 0;
    int32_t protected_cell = 0;
    int32_t dwl = 0;
    int32_t dhl = 0;
};

struct scrollrect_record {
    Rect rect;
    int32_t downward = 0;
    int32_t rightward = 0;
};

struct moverect_record {
    Rect dest;
    Rect src;
};

struct premove_record {
    Rect rect;
};

struct erase_record {
    Rect rect;
    bool selective = false;
};

struct damage_record {
    Rect rect;
};

struct sb_pushline_record {
    int32_t cols = 0;
    bool continuation = false;
    std::array<uint32_t, 256> chars{};
};

struct settermprop_record {
    Prop prop{};
    Value val{};
};

struct movecursor_record {
    Pos pos;
    Pos oldpos;
    bool visible = false;
};

struct CallbackLog {
    std::array<putglyph_record, CALLBACK_LOG_MAX> putglyph{};
    int32_t putglyph_count = 0;

    std::array<scrollrect_record, CALLBACK_LOG_MAX> scrollrect{};
    int32_t scrollrect_count = 0;

    std::array<moverect_record, CALLBACK_LOG_MAX> moverect{};
    int32_t moverect_count = 0;

    std::array<erase_record, CALLBACK_LOG_MAX> erase{};
    int32_t erase_count = 0;

    std::array<premove_record, CALLBACK_LOG_MAX> premove{};
    int32_t premove_count = 0;

    std::array<damage_record, CALLBACK_LOG_MAX> damage{};
    int32_t damage_count = 0;

    std::array<sb_pushline_record, CALLBACK_LOG_MAX> sb_pushline{};
    int32_t sb_pushline_count = 0;

    int32_t sb_popline_count = 0;

    std::array<settermprop_record, CALLBACK_LOG_MAX> settermprop{};
    int32_t settermprop_count = 0;

    std::array<movecursor_record, CALLBACK_LOG_MAX> movecursor{};
    int32_t movecursor_count = 0;

    int32_t sb_clear_count = 0;
    int32_t bell_count = 0;
};
inline CallbackLog g_cb;

inline void callbacks_clear() {
    g_cb = {};
}

// --- State callbacks as ABC subclass ---

struct TestStateCallbacks : StateCallbacks {
    bool on_putglyph(const GlyphInfo& info, Pos pos) override {
        if(g_cb.putglyph_count < CALLBACK_LOG_MAX) {
            auto& r = g_cb.putglyph[g_cb.putglyph_count];
            for(int32_t i = 0; i < 6 && i < static_cast<int32_t>(info.chars.size()); i++)
                r.chars[i] = info.chars[i];
            r.width = info.width;
            r.row = pos.row;
            r.col = pos.col;
            r.protected_cell = info.protected_cell;
            r.dwl = info.dwl;
            r.dhl = info.dhl;
        }
        g_cb.putglyph_count++;
        return true;
    }

    bool on_movecursor(Pos pos, Pos oldpos, bool visible) override {
        if(g_cb.movecursor_count < CALLBACK_LOG_MAX) {
            auto& r = g_cb.movecursor[g_cb.movecursor_count];
            r.pos = pos;
            r.oldpos = oldpos;
            r.visible = visible;
        }
        g_cb.movecursor_count++;
        return true;
    }

    bool on_scrollrect(Rect rect, int32_t downward, int32_t rightward) override {
        if(g_cb.scrollrect_count < CALLBACK_LOG_MAX) {
            auto& r = g_cb.scrollrect[g_cb.scrollrect_count];
            r.rect = rect;
            r.downward = downward;
            r.rightward = rightward;
        }
        g_cb.scrollrect_count++;
        return true;
    }

    bool on_moverect(Rect dest, Rect src) override {
        if(g_cb.moverect_count < CALLBACK_LOG_MAX) {
            auto& r = g_cb.moverect[g_cb.moverect_count];
            r.dest = dest;
            r.src = src;
        }
        g_cb.moverect_count++;
        return true;
    }

    bool on_erase(Rect rect, bool selective) override {
        if(g_cb.erase_count < CALLBACK_LOG_MAX) {
            auto& r = g_cb.erase[g_cb.erase_count];
            r.rect = rect;
            r.selective = selective;
        }
        g_cb.erase_count++;
        return true;
    }

    bool on_settermprop(Prop prop, const Value& val) override {
        if(g_cb.settermprop_count < CALLBACK_LOG_MAX) {
            auto& r = g_cb.settermprop[g_cb.settermprop_count];
            r.prop = prop;
            r.val = val;
        }
        g_cb.settermprop_count++;
        return true;
    }

    bool on_bell() override {
        g_cb.bell_count++;
        return true;
    }

    bool on_sb_clear() override {
        g_cb.sb_clear_count++;
        return true;
    }

    bool on_premove(Rect dest) override {
        if(g_cb.premove_count < CALLBACK_LOG_MAX) {
            auto& r = g_cb.premove[g_cb.premove_count];
            r.rect = dest;
        }
        g_cb.premove_count++;
        return true;
    }

    bool on_setlineinfo([[maybe_unused]] int32_t row, [[maybe_unused]] const LineInfo& newinfo, [[maybe_unused]] const LineInfo& oldinfo) override {
        return true;
    }
};

// scrollrect returning false (not handled) — triggers moverect+erase fallback
struct TestStateCallbacksNoScrollrect : TestStateCallbacks {
    bool on_scrollrect([[maybe_unused]] Rect rect, [[maybe_unused]] int32_t downward, [[maybe_unused]] int32_t rightward) override {
        return false;
    }
};

inline TestStateCallbacks state_cbs;
inline TestStateCallbacksNoScrollrect state_cbs_no_scrollrect;

// --- Screen callbacks as ABC subclass ---

struct TestScreenCallbacks : ScreenCallbacks {
    bool on_damage(Rect rect) override {
        if(g_cb.damage_count < CALLBACK_LOG_MAX)
            g_cb.damage[g_cb.damage_count].rect = rect;
        g_cb.damage_count++;
        return true;
    }

    bool on_moverect(Rect dest, Rect src) override {
        if(g_cb.moverect_count < CALLBACK_LOG_MAX) {
            auto& r = g_cb.moverect[g_cb.moverect_count];
            r.dest = dest;
            r.src = src;
        }
        g_cb.moverect_count++;
        return true;
    }

    bool on_movecursor(Pos pos, Pos oldpos, bool visible) override {
        if(g_cb.movecursor_count < CALLBACK_LOG_MAX) {
            auto& r = g_cb.movecursor[g_cb.movecursor_count];
            r.pos = pos;
            r.oldpos = oldpos;
            r.visible = visible;
        }
        g_cb.movecursor_count++;
        return true;
    }

    bool on_settermprop(Prop prop, const Value& val) override {
        if(g_cb.settermprop_count < CALLBACK_LOG_MAX) {
            auto& r = g_cb.settermprop[g_cb.settermprop_count];
            r.prop = prop;
            r.val = val;
        }
        g_cb.settermprop_count++;
        return true;
    }

    bool on_bell() override {
        g_cb.bell_count++;
        return true;
    }

    bool on_sb_pushline(std::span<const ScreenCell> cells, bool continuation) override {
        if(g_cb.sb_pushline_count < CALLBACK_LOG_MAX) {
            auto& r = g_cb.sb_pushline[g_cb.sb_pushline_count];
            r.cols = static_cast<int32_t>(cells.size());
            r.continuation = continuation;
            for(int32_t i = 0; i < static_cast<int32_t>(cells.size()) && i < 256; i++)
                r.chars[i] = cells[i].chars[0];
        }
        g_cb.sb_pushline_count++;
        return true;
    }

    bool on_sb_popline([[maybe_unused]] std::span<ScreenCell> cells, bool& continuation) override {
        continuation = false;
        g_cb.sb_popline_count++;
        return false; // no scrollback available
    }
};

inline TestScreenCallbacks screen_cbs;

// --- Parser callback recording ---

inline constexpr int32_t PARSER_TEXT_MAX = 1024;
inline constexpr int32_t PARSER_LOG_MAX = 64;

struct parser_text_record {
    std::array<char, PARSER_TEXT_MAX> bytes{};
    size_t len = 0;
};

struct parser_control_record {
    uint8_t control = 0;
};

struct parser_escape_record {
    std::array<char, 32> seq{};
    size_t len = 0;
};

struct parser_csi_record {
    char command = 0;
    std::array<int64_t, 16> args{};
    int32_t argcount = 0;
    std::array<char, 4> leader{};
    std::array<char, 4> intermed{};
};

struct parser_osc_record {
    int32_t command = 0;
    std::array<char, 256> data{};
    size_t datalen = 0;
    bool initial = false;
    bool final_ = false;
};

struct ParserLog {
    std::array<parser_text_record, PARSER_LOG_MAX> text{};
    int32_t text_count = 0;

    std::array<parser_control_record, PARSER_LOG_MAX> control{};
    int32_t control_count = 0;

    std::array<parser_escape_record, PARSER_LOG_MAX> escape{};
    int32_t escape_count = 0;

    std::array<parser_csi_record, PARSER_LOG_MAX> csi{};
    int32_t csi_count = 0;

    std::array<parser_osc_record, PARSER_LOG_MAX> osc{};
    int32_t osc_count = 0;
};
inline ParserLog g_parser;

inline void parser_clear() {
    g_parser = {};
}

struct TestParserCallbacks : ParserCallbacks {
    int32_t on_text(std::span<const char> bytes) override {
        size_t len = bytes.size();
        size_t i;
        for(i = 0; i < len; i++) {
            uint8_t b = static_cast<uint8_t>(bytes[i]);
            if(b < 0x20 || b == 0x7f || (b >= 0x80 && b < 0xa0))
                break;
        }
        if(g_parser.text_count < PARSER_LOG_MAX) {
            auto& r = g_parser.text[g_parser.text_count];
            size_t store = i < PARSER_TEXT_MAX ? i : PARSER_TEXT_MAX;
            std::copy_n(bytes.data(), store, r.bytes.data());
            r.len = i;
        }
        g_parser.text_count++;
        return static_cast<int32_t>(i);
    }

    bool on_control(uint8_t control) override {
        if(g_parser.control_count < PARSER_LOG_MAX)
            g_parser.control[g_parser.control_count].control = control;
        g_parser.control_count++;
        return true;
    }

    bool on_escape(std::string_view bytes) override {
        if(g_parser.escape_count < PARSER_LOG_MAX) {
            auto& r = g_parser.escape[g_parser.escape_count];
            size_t len = bytes.size();
            if(len > r.seq.size() - 1) len = r.seq.size() - 1;
            std::copy_n(bytes.data(), len, r.seq.data());
            r.seq[len] = '\0';
            r.len = len;
        }
        g_parser.escape_count++;
        return true;
    }

    bool on_csi(std::string_view leader, std::span<const int64_t> args, std::string_view intermed, char command) override {
        if(g_parser.csi_count < PARSER_LOG_MAX) {
            auto& r = g_parser.csi[g_parser.csi_count];
            int32_t argcount = static_cast<int32_t>(args.size());
            r.command = command;
            r.argcount = argcount < 16 ? argcount : 16;
            for(int32_t i = 0; i < r.argcount; i++)
                r.args[i] = args[i];
            if(!leader.empty()) { size_t n = std::min(leader.size(), r.leader.size() - 1); std::copy_n(leader.data(), n, r.leader.data()); r.leader[n] = '\0'; }
            else r.leader[0] = '\0';
            if(!intermed.empty()) { size_t n = std::min(intermed.size(), r.intermed.size() - 1); std::copy_n(intermed.data(), n, r.intermed.data()); r.intermed[n] = '\0'; }
            else r.intermed[0] = '\0';
        }
        g_parser.csi_count++;
        return true;
    }

    bool on_osc(int32_t command, StringFragment frag) override {
        if(frag.initial) {
            if(g_parser.osc_count < PARSER_LOG_MAX) {
                auto& r = g_parser.osc[g_parser.osc_count];
                r.command = command;
                r.datalen = 0;
                r.initial = true;
                r.final_ = false;
            }
        }
        if(g_parser.osc_count < PARSER_LOG_MAX) {
            auto& r = g_parser.osc[g_parser.osc_count];
            if(r.datalen + frag.str.size() <= r.data.size())
                std::copy_n(frag.str.data(), frag.str.size(), r.data.data() + r.datalen);
            r.datalen += frag.str.size();
            if(frag.final_) {
                r.final_ = true;
                g_parser.osc_count++;
            }
        }
        return true;
    }
};

inline TestParserCallbacks parser_cbs;

// ============================================================================
// Fallback callback recording
// ============================================================================

inline constexpr int32_t FALLBACK_LOG_MAX = 16;

struct fallback_control_record { uint8_t control = 0; };
struct fallback_csi_record {
    char command = 0;
    std::array<int64_t, 16> args{};
    int32_t argcount = 0;
    std::array<char, 8> leader{};
    std::array<char, 8> intermed{};
};
struct fallback_osc_record {
    int32_t command = 0;
    std::array<char, 256> data{};
    size_t datalen = 0;
};
struct fallback_string_record {
    std::array<char, 256> data{};
    size_t datalen = 0;
};

struct FallbackLog {
    std::array<fallback_control_record, FALLBACK_LOG_MAX> control{};
    int32_t control_count = 0;
    std::array<fallback_csi_record, FALLBACK_LOG_MAX> csi{};
    int32_t csi_count = 0;
    std::array<fallback_osc_record, FALLBACK_LOG_MAX> osc{};
    int32_t osc_count = 0;
    std::array<fallback_string_record, FALLBACK_LOG_MAX> dcs{};
    int32_t dcs_count = 0;
    std::array<fallback_string_record, FALLBACK_LOG_MAX> apc{};
    int32_t apc_count = 0;
    std::array<fallback_string_record, FALLBACK_LOG_MAX> pm{};
    int32_t pm_count = 0;
    std::array<fallback_string_record, FALLBACK_LOG_MAX> sos{};
    int32_t sos_count = 0;
};
inline FallbackLog g_fallback;

inline void fallback_clear() {
    g_fallback = {};
}

struct TestStateFallbacks : StateFallbacks {
    bool on_control(uint8_t control) override {
        if(g_fallback.control_count < FALLBACK_LOG_MAX)
            g_fallback.control[g_fallback.control_count].control = control;
        g_fallback.control_count++;
        return false;
    }

    bool on_csi(std::string_view leader, std::span<const int64_t> args, std::string_view intermed, char command) override {
        if(g_fallback.csi_count < FALLBACK_LOG_MAX) {
            auto& r = g_fallback.csi[g_fallback.csi_count];
            r.command = command;
            int32_t argcount = static_cast<int32_t>(args.size());
            r.argcount = argcount < 16 ? argcount : 16;
            for(int32_t i = 0; i < r.argcount; i++)
                r.args[i] = args[i];
            if(!leader.empty()) { size_t n = std::min(leader.size(), r.leader.size() - 1); std::copy_n(leader.data(), n, r.leader.data()); r.leader[n] = '\0'; }
            else r.leader[0] = '\0';
            if(!intermed.empty()) { size_t n = std::min(intermed.size(), r.intermed.size() - 1); std::copy_n(intermed.data(), n, r.intermed.data()); r.intermed[n] = '\0'; }
            else r.intermed[0] = '\0';
        }
        g_fallback.csi_count++;
        return false;
    }

    bool on_osc(int32_t command, StringFragment frag) override {
        if(frag.initial && g_fallback.osc_count < FALLBACK_LOG_MAX) {
            auto& r = g_fallback.osc[g_fallback.osc_count];
            r.command = command;
            r.datalen = 0;
        }
        if(g_fallback.osc_count < FALLBACK_LOG_MAX) {
            auto& r = g_fallback.osc[g_fallback.osc_count];
            if(r.datalen + frag.str.size() <= r.data.size())
                std::copy_n(frag.str.data(), frag.str.size(), r.data.data() + r.datalen);
            r.datalen += frag.str.size();
            if(frag.final_)
                g_fallback.osc_count++;
        }
        return false;
    }

    bool on_dcs(std::string_view command, StringFragment frag) override {
        if(frag.initial && g_fallback.dcs_count < FALLBACK_LOG_MAX) {
            auto& r = g_fallback.dcs[g_fallback.dcs_count];
            r.datalen = 0;
            if(command.size() <= r.data.size()) {
                std::copy_n(command.data(), command.size(), r.data.data());
                r.datalen = command.size();
            }
        }
        if(g_fallback.dcs_count < FALLBACK_LOG_MAX) {
            auto& r = g_fallback.dcs[g_fallback.dcs_count];
            if(r.datalen + frag.str.size() <= r.data.size())
                std::copy_n(frag.str.data(), frag.str.size(), r.data.data() + r.datalen);
            r.datalen += frag.str.size();
            if(frag.final_)
                g_fallback.dcs_count++;
        }
        return false;
    }

    bool on_apc(StringFragment frag) override {
        if(frag.initial && g_fallback.apc_count < FALLBACK_LOG_MAX)
            g_fallback.apc[g_fallback.apc_count].datalen = 0;
        if(g_fallback.apc_count < FALLBACK_LOG_MAX) {
            auto& r = g_fallback.apc[g_fallback.apc_count];
            if(r.datalen + frag.str.size() <= r.data.size())
                std::copy_n(frag.str.data(), frag.str.size(), r.data.data() + r.datalen);
            r.datalen += frag.str.size();
            if(frag.final_)
                g_fallback.apc_count++;
        }
        return false;
    }

    bool on_pm(StringFragment frag) override {
        if(frag.initial && g_fallback.pm_count < FALLBACK_LOG_MAX)
            g_fallback.pm[g_fallback.pm_count].datalen = 0;
        if(g_fallback.pm_count < FALLBACK_LOG_MAX) {
            auto& r = g_fallback.pm[g_fallback.pm_count];
            if(r.datalen + frag.str.size() <= r.data.size())
                std::copy_n(frag.str.data(), frag.str.size(), r.data.data() + r.datalen);
            r.datalen += frag.str.size();
            if(frag.final_)
                g_fallback.pm_count++;
        }
        return false;
    }

    bool on_sos(StringFragment frag) override {
        if(frag.initial && g_fallback.sos_count < FALLBACK_LOG_MAX)
            g_fallback.sos[g_fallback.sos_count].datalen = 0;
        if(g_fallback.sos_count < FALLBACK_LOG_MAX) {
            auto& r = g_fallback.sos[g_fallback.sos_count];
            if(r.datalen + frag.str.size() <= r.data.size())
                std::copy_n(frag.str.data(), frag.str.size(), r.data.data() + r.datalen);
            r.datalen += frag.str.size();
            if(frag.final_)
                g_fallback.sos_count++;
        }
        return false;
    }
};

inline TestStateFallbacks fallback_cbs;

// ============================================================================
// Screen callbacks with scrollback
// ============================================================================

struct TestScreenCallbacksScrollback : TestScreenCallbacks {
    bool on_sb_pushline(std::span<const ScreenCell> cells, bool continuation) override {
        int32_t cols = static_cast<int32_t>(cells.size());
        if(g_cb.sb_pushline_count < CALLBACK_LOG_MAX) {
            auto& r = g_cb.sb_pushline[g_cb.sb_pushline_count];
            r.cols = cols;
            r.continuation = continuation;
            for(int32_t i = 0; i < cols && i < 256; i++)
                r.chars[i] = cells[i].chars[0];
        }
        g_cb.sb_pushline_count++;
        return true;
    }

    bool on_sb_popline(std::span<ScreenCell> cells, bool& continuation) override {
        int32_t cols = static_cast<int32_t>(cells.size());
        for(int32_t col = 0; col < cols; col++) {
            cells[col] = {};
            if(col < 5)
                cells[col].chars[0] = 'A' + col;
            cells[col].width = 1;
        }
        continuation = false;
        g_cb.sb_popline_count++;
        return true;
    }
};

inline TestScreenCallbacksScrollback screen_cbs_scrollback;

// ============================================================================
// Screen callbacks with real scrollback store (for reflow tests)
// ============================================================================

inline constexpr int32_t SCROLLBACK_MAX = 64;
inline constexpr int32_t SCROLLBACK_COLS_MAX = 256;

struct scrollback_line {
    std::array<ScreenCell, SCROLLBACK_COLS_MAX> cells{};
    int32_t cols = 0;
    bool continuation = false;
};

inline std::array<scrollback_line, SCROLLBACK_MAX> g_scrollback{};
inline int32_t g_scrollback_count = 0;

inline void scrollback_clear() {
    g_scrollback = {};
    g_scrollback_count = 0;
}

struct TestScreenCallbacksScrollbackReflow : TestScreenCallbacks {
    bool on_sb_pushline(std::span<const ScreenCell> cells, bool continuation) override {
        int32_t cols = static_cast<int32_t>(cells.size());
        if(g_scrollback_count < SCROLLBACK_MAX) {
            auto& line = g_scrollback[g_scrollback_count];
            int32_t n = cols < SCROLLBACK_COLS_MAX ? cols : SCROLLBACK_COLS_MAX;
            std::copy_n(cells.data(), n, line.cells.data());
            line.cols = n;
            line.continuation = continuation;
            g_scrollback_count++;
        }
        if(g_cb.sb_pushline_count < CALLBACK_LOG_MAX) {
            auto& r = g_cb.sb_pushline[g_cb.sb_pushline_count];
            r.cols = cols;
            r.continuation = continuation;
            for(int32_t i = 0; i < cols && i < 256; i++)
                r.chars[i] = cells[i].chars[0];
        }
        g_cb.sb_pushline_count++;
        return true;
    }

    bool on_sb_popline(std::span<ScreenCell> cells, bool& continuation) override {
        int32_t cols = static_cast<int32_t>(cells.size());
        if(g_scrollback_count <= 0)
            return false;
        g_scrollback_count--;
        auto& line = g_scrollback[g_scrollback_count];
        int32_t n = cols < line.cols ? cols : line.cols;
        std::copy_n(line.cells.data(), n, cells.data());
        for(int32_t i = n; i < cols; i++) {
            cells[i] = {};
            cells[i].width = 1;
        }
        continuation = line.continuation;
        g_cb.sb_popline_count++;
        return true;
    }
};

inline TestScreenCallbacksScrollbackReflow screen_cbs_scrollback_reflow;

// ============================================================================
// Encoding callback recording
// ============================================================================

inline constexpr int32_t ENCODING_OUT_MAX = 256;

struct EncodingLog {
    std::array<uint32_t, ENCODING_OUT_MAX> codepoints{};
    int32_t count = 0;
};
inline EncodingLog g_encoding;

inline void encoding_clear() {
    g_encoding = {};
}

#endif // HARNESS_H
