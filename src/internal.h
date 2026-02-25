#ifndef VTERM_INTERNAL_H
#define VTERM_INTERNAL_H

#include "vterm/vterm.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <concepts>
#include <cstdlib>
#include <format>
#include <functional>
#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#ifdef DEBUG
# include <iostream>
# define DEBUG_LOG(fmt, ...) std::cerr << std::format(fmt __VA_OPT__(,) __VA_ARGS__)
#else
# define DEBUG_LOG(...)
#endif

namespace vterm {

inline constexpr char esc_c = '\x1b';

// Helper to construct a span from a pointer and int32_t count,
// avoiding repeated static_cast<size_t> at every call site.
template<typename T>
    requires std::is_object_v<T>
[[nodiscard]] constexpr std::span<T> make_span(T* data, int32_t count) {
    assert(count >= 0);
    return {data, static_cast<size_t>(count)};
}

inline constexpr int32_t intermed_max   = 16;
inline constexpr int32_t csi_args_max   = 16;
inline constexpr int32_t csi_leader_max = 16;

// High bit mask — distinguishes GR (high) from GL (low) byte range
inline constexpr uint8_t high_bit = 0x80;

// Offset between 8-bit C1 control codes (0x80-0x9F) and their
// 7-bit ESC-prefixed equivalents (ESC 0x40-0x5F)
inline constexpr uint8_t c1_esc_offset = 0x40;

inline constexpr int32_t bufidx_primary   = 0;
inline constexpr int32_t bufidx_altscreen = 1;

// Sentinel value for "no damage" / "no pending scroll" in Screen::Impl
inline constexpr int32_t no_damage_row = -1;

// Sentinel for "not set" scroll region boundaries in State::Impl
inline constexpr int32_t scrollregion_unset = -1;

// Sentinel for "not yet determined" cursor position during resize
inline constexpr int32_t cursor_unset = -1;

// Initial size of the combining character buffer in State::Impl
inline constexpr int32_t initial_combine_size = 16;

// --- Color palette ---

inline constexpr int32_t palette_normal_count = 8;
inline constexpr int32_t palette_ansi_count   = 16;
inline constexpr int32_t palette_cube_end     = 232;
inline constexpr int32_t palette_max          = 256;

inline constexpr std::array<uint8_t, 6> ramp6 = {{
    0x00, 0x33, 0x66, 0x99, 0xCC, 0xFF,
}};

inline constexpr std::array<uint8_t, 24> ramp24 = {{
    0x00, 0x0B, 0x16, 0x21, 0x2C, 0x37, 0x42, 0x4D, 0x58, 0x63, 0x6E, 0x79,
    0x85, 0x90, 0x9B, 0xA6, 0xB1, 0xBC, 0xC7, 0xD2, 0xDD, 0xE8, 0xF3, 0xFF,
}};

// --- Encoding ---

enum class EncodingType {
    UTF8,
    Single94,
};

struct DecodeResult {
    int32_t codepoints_produced = 0;
    size_t bytes_consumed = 0;
};

struct EncodingInstance {
    virtual ~EncodingInstance() = default;
    EncodingInstance(const EncodingInstance&) = delete;
    EncodingInstance& operator=(const EncodingInstance&) = delete;
    EncodingInstance() = default;
    virtual void init() {}
    virtual DecodeResult decode(std::span<uint32_t> output, std::span<const char> input) = 0;
};

[[nodiscard]] std::unique_ptr<EncodingInstance> create_encoding(EncodingType type, char designation);

// --- Pen (internal) ---

struct Pen {
    Color fg{};
    Color bg{};
    uint32_t bold      : 1 = 0;
    Underline underline : 2 = Underline::Off;
    uint32_t italic    : 1 = 0;
    uint32_t blink     : 1 = 0;
    uint32_t reverse   : 1 = 0;
    uint32_t conceal   : 1 = 0;
    uint32_t strike    : 1 = 0;
    uint32_t font      : 4 = 0;
    uint32_t small     : 1 = 0;
    Baseline baseline   : 2 = Baseline::Normal;
};

// --- C0 control codes ---

inline constexpr uint8_t ctrl_nul = 0x00;
inline constexpr uint8_t ctrl_bel = 0x07;
inline constexpr uint8_t ctrl_bs  = 0x08;
inline constexpr uint8_t ctrl_ht  = 0x09;
inline constexpr uint8_t ctrl_lf  = 0x0a;
inline constexpr uint8_t ctrl_vt  = 0x0b;
inline constexpr uint8_t ctrl_ff  = 0x0c;
inline constexpr uint8_t ctrl_cr  = 0x0d;
inline constexpr uint8_t ctrl_ls1 = 0x0e;
inline constexpr uint8_t ctrl_ls0 = 0x0f;
inline constexpr uint8_t ctrl_can = 0x18;
inline constexpr uint8_t ctrl_sub = 0x1a;
inline constexpr uint8_t ctrl_esc = 0x1b;
inline constexpr uint8_t ctrl_del = 0x7f;

// First non-C0 printable byte
inline constexpr uint8_t c0_end   = 0x20;

// C1 control code range (8-bit)
inline constexpr uint8_t c1_start = 0x80;
inline constexpr uint8_t c1_end   = 0xa0;

// --- C1 control codes ---

inline constexpr uint8_t ctrl_ind = 0x84;
inline constexpr uint8_t ctrl_nel = 0x85;
inline constexpr uint8_t ctrl_hts = 0x88;
inline constexpr uint8_t ctrl_ri  = 0x8d;
inline constexpr uint8_t ctrl_ss2 = 0x8e;
inline constexpr uint8_t ctrl_ss3 = 0x8f;

enum class C1 : uint8_t {
    None = 0x00,
    SS3 = 0x8f,
    DCS = 0x90,
    SOS = 0x98,
    CSI = 0x9b,
    ST  = 0x9c,
    OSC = 0x9d,
    PM  = 0x9e,
    APC = 0x9f,
};

// --- Tabstop bit packing (8 columns per byte) ---

inline constexpr int32_t tabstop_byte_shift    = 3;  // col >> 3 == col / 8
inline constexpr int32_t tabstop_bit_mask      = 7;  // col & 7 == col % 8
inline constexpr int32_t default_tabstop_interval = 8;

// --- Mouse flags ---

inline constexpr int32_t mouse_want_click = 0x01;
inline constexpr int32_t mouse_want_drag  = 0x02;
inline constexpr int32_t mouse_want_move  = 0x04;

// --- Parser state ---

enum class ParserState {
    Normal,
    CSILeader,
    CSIArgs,
    CSIIntermed,
    DCSCommand,
    // Below here are the "string states" — is_string_state() relies on this ordering
    OSCCommand,
    OSC,
    DCS,
    APC,
    PM,
    SOS,
};

static_assert(ParserState::OSCCommand > ParserState::DCSCommand,
    "is_string_state() relies on string states being after DCSCommand");

// --- Selection state ---

enum class SelectionState : uint8_t {
    Initial,
    Selected,
    Query,
    SetInitial,
    Set,
    Invalid,
};

// --- Mouse protocol ---

enum class MouseProtocol { X10, UTF8, SGR, RXVT };

// --- State::Impl ---

struct State::Impl {
    explicit Impl(Terminal::Impl& vt_ref);
    ~Impl(); // defined in state.cpp

    Terminal::Impl& vt;

    // Owned parser callbacks — created by obtain_state, destroyed with State::Impl
    std::unique_ptr<ParserCallbacks> owned_parser_callbacks;

    StateCallbacks* callbacks = nullptr;
    bool callbacks_has_premove = false;

    StateFallbacks* fallbacks = nullptr;

    int32_t rows = 0;
    int32_t cols = 0;

    Pos pos;

    bool at_phantom = false;

    int32_t scrollregion_top = 0;
    int32_t scrollregion_bottom = scrollregion_unset;
    int32_t scrollregion_left = 0;
    int32_t scrollregion_right = scrollregion_unset;

    std::vector<uint8_t> tabstops;

    std::array<std::vector<LineInfo>, 2> lineinfos{};
    int32_t lineinfo_bufidx = bufidx_primary;

    LineInfo& get_lineinfo(int32_t row) {
        assert(row >= 0 && row < static_cast<int32_t>(lineinfos[lineinfo_bufidx].size()));
        return lineinfos[lineinfo_bufidx][row];
    }
    const LineInfo& get_lineinfo(int32_t row) const {
        assert(row >= 0 && row < static_cast<int32_t>(lineinfos[lineinfo_bufidx].size()));
        return lineinfos[lineinfo_bufidx][row];
    }

    // Mouse state
    int32_t mouse_col = 0, mouse_row = 0;
    int32_t mouse_buttons = 0;
    int32_t mouse_flags = 0;
    MouseProtocol mouse_protocol = MouseProtocol::X10;

    // Combining state
    std::vector<uint32_t> combine_chars;
    int32_t combine_width = 0;
    int32_t combine_count = 0;
    Pos combine_pos;

    struct {
        uint32_t keypad        : 1 = 0;
        uint32_t cursor        : 1 = 0;
        uint32_t autowrap      : 1 = 0;
        uint32_t insert        : 1 = 0;
        uint32_t newline       : 1 = 0;
        uint32_t cursor_visible: 1 = 0;
        uint32_t cursor_blink  : 1 = 0;
        uint32_t cursor_shape  : 2 = 0;
        uint32_t alt_screen    : 1 = 0;
        uint32_t origin        : 1 = 0;
        uint32_t screen        : 1 = 0;
        uint32_t leftrightmargin: 1 = 0;
        uint32_t bracketpaste  : 1 = 0;
        uint32_t report_focus  : 1 = 0;
    } mode{};

    std::array<std::unique_ptr<EncodingInstance>, 4> encoding;
    std::unique_ptr<EncodingInstance> encoding_utf8;
    int32_t gl_set = 0, gr_set = 0, gsingle_set = 0;

    Pen pen = {};

    Color default_fg{};
    Color default_bg{};
    std::array<Color, 16> colors = {};

    bool bold_is_highbright = false;

    uint32_t protected_cell : 1 = 0;

    struct {
        Pos pos;
        Pen pen;
        struct {
            uint32_t    cursor_visible : 1 = 0;
            uint32_t    cursor_blink   : 1 = 0;
            uint32_t    cursor_shape   : 2 = 0;
        } mode;
    } saved = {};

    union {
        std::array<char, 4> decrqss;
        struct {
            uint16_t mask;
            SelectionState state;
            uint32_t recvpartial;
            uint32_t sendpartial;
        } selection;
    } tmp = {};

    struct {
        SelectionCallbacks* callbacks = nullptr;
        std::vector<char> buffer;
    } selection;

    [[nodiscard]] SelectionMask selection_mask() const { return static_cast<SelectionMask>(tmp.selection.mask); }
    [[nodiscard]] int32_t scrollregion_bottom_val() const { return scrollregion_bottom != scrollregion_unset ? scrollregion_bottom : rows; }
    [[nodiscard]] int32_t scrollregion_left_val() const { return mode.leftrightmargin ? scrollregion_left : 0; }
    [[nodiscard]] int32_t scrollregion_right_val() const { return (mode.leftrightmargin && scrollregion_right != scrollregion_unset) ? scrollregion_right : cols; }
    [[nodiscard]] int32_t row_width(int32_t row) const { return get_lineinfo(row).doublewidth ? (cols / 2) : cols; }
    [[nodiscard]] int32_t this_row_width() const { return row_width(pos.row); }

    // Methods (defined in pen.cpp / state.cpp)
    void newpen();
    void resetpen();
    void setpen(std::span<const int64_t> args);
    [[nodiscard]] int32_t getpen(std::span<int64_t> args);
    void savepen(bool save);
    void reset(bool hard);

    // Pen helpers (defined in pen.cpp)
    [[nodiscard]] bool lookup_colour_ansi(int64_t index, Color& col) const;
    [[nodiscard]] bool lookup_colour_palette(int64_t index, Color& col) const;
    [[nodiscard]] int32_t lookup_colour(int32_t palette, std::span<const int64_t> args, Color& col) const;
    void setpenattr(Attr attr, ValueType type, const Value& val);
    void setpenattr_bool(Attr attr, bool boolean);
    void setpenattr_int(Attr attr, int32_t number);
    void setpenattr_col(Attr attr, Color color);
    void set_pen_col_ansi(Attr attr, int64_t col);

    // State callbacks (defined in state.cpp)
    void putglyph(std::span<const uint32_t> chars, int32_t width, Pos pos);
    void updatecursor(const Pos& oldpos, bool cancel_phantom);
    void erase(Rect rect, bool selective);
    void scroll(Rect rect, int32_t downward, int32_t rightward);
    void linefeed();
    void grow_combine_buffer();
    void set_col_tabstop(int32_t col);
    void clear_col_tabstop(int32_t col);
    [[nodiscard]] bool is_col_tabstop(int32_t col) const;
    [[nodiscard]] bool is_cursor_in_scrollregion() const;
    void tab(int32_t count, int32_t direction);
    enum class DWL : int32_t { Off = 0, On = 1 };
    enum class DHL : int32_t { Off = 0, Top = 1, Bottom = 2 };
    void set_lineinfo(int32_t row, bool force, DWL dwl, DHL dhl);
    [[nodiscard]] bool settermprop_bool(Prop prop, bool v);
    [[nodiscard]] bool settermprop_int(Prop prop, int32_t v);
    [[nodiscard]] bool settermprop_string(Prop prop, StringFragment frag);
    [[nodiscard]] bool set_termprop_internal(Prop prop, const Value& val);
    void savecursor(bool save);
    int32_t on_text(std::span<const char> bytes);
    [[nodiscard]] bool on_control(uint8_t control);
    [[nodiscard]] bool on_escape(std::string_view bytes);
    [[nodiscard]] bool on_csi(std::string_view leader, std::span<const int64_t> args, std::string_view intermed, char command);
    [[nodiscard]] bool on_osc(int32_t command, StringFragment frag);
    [[nodiscard]] bool on_dcs(std::string_view command, StringFragment frag);
    [[nodiscard]] bool on_apc(StringFragment frag);
    [[nodiscard]] bool on_pm(StringFragment frag);
    [[nodiscard]] bool on_sos(StringFragment frag);
    [[nodiscard]] bool on_resize(int32_t rows, int32_t cols);
    void set_mode(int32_t num, bool val);
    void set_dec_mode(int32_t num, bool val);
    void request_dec_mode(int32_t num);
    void request_version_string();
    void osc_selection(StringFragment frag);
    void request_status_string(StringFragment frag);

    void output_mouse(int32_t code, bool pressed, int32_t modifiers, int32_t col, int32_t row);
};

// --- Terminal::Impl ---

struct Terminal::Impl {
    Impl();  // defined in screen.cpp (needs complete Screen::Impl)
    ~Impl(); // defined in screen.cpp (needs complete Screen::Impl)

    int32_t rows = 0;
    int32_t cols = 0;

    struct {
        uint32_t utf8     : 1 = 0;
        uint32_t ctrl8bit : 1 = 0;
    } mode{};

    struct {
        ParserState state = ParserState::Normal;

        bool in_esc = false;

        size_t intermedlen = 0;
        std::array<char, intermed_max> intermed = {};

        union {
            struct {
                size_t leaderlen;
                std::array<char, csi_leader_max> leader;
                int32_t argi;
                std::array<int64_t, csi_args_max> args;
            } csi;
            struct {
                int32_t command;
            } osc;
            struct {
                size_t commandlen;
                std::array<char, csi_leader_max> command;
            } dcs;
        } v = {};

        ParserCallbacks* callbacks = nullptr;

        bool string_initial = false;
        bool emit_nul = false;
    } parser;

    std::function<void(std::span<const char>)> outfunc;

    // Fixed-size output buffer — intentionally does not grow; data is silently
    // dropped when full, matching the original C library behaviour.
    std::vector<char> outbuffer;
    size_t outbuffer_cur = 0;

    std::unique_ptr<State::Impl> state;
    std::unique_ptr<Screen::Impl> screen;

    // Wrapper objects returned by Terminal::state() / Terminal::screen()
    State  state_wrapper;
    Screen screen_wrapper;

    // Output
    void push_output_bytes(std::span<const char> bytes);

    void append_c1(std::string& s, C1 ctrl) {
        auto byte = to_underlying(ctrl);
        if(byte >= high_bit && !mode.ctrl8bit) {
            s += esc_c;
            s += static_cast<char>(byte - c1_esc_offset);
        }
        else {
            s += static_cast<char>(byte);
        }
    }

    template<typename... Args>
    void push_output(std::format_string<Args...> fmt, Args&&... args) {
        auto s = std::format(fmt, std::forward<Args>(args)...);
        push_output_bytes(s);
    }

    template<typename... Args>
    void push_output_ctrl(C1 ctrl, std::format_string<Args...> fmt, Args&&... args) {
        std::string s;
        append_c1(s, ctrl);
        s += std::format(fmt, std::forward<Args>(args)...);
        push_output_bytes(s);
    }

    template<typename... Args>
    void push_output_str(C1 ctrl, bool term, std::format_string<Args...> fmt, Args&&... args) {
        std::string s;
        if(ctrl != C1::None)
            append_c1(s, ctrl);
        s += std::format(fmt, std::forward<Args>(args)...);
        if(term) {
            if(mode.ctrl8bit)
                s += static_cast<char>(to_underlying(C1::ST));
            else {
                s += esc_c;
                s += '\\';
            }
        }
        push_output_bytes(s);
    }

    // Parser
    size_t input_write(std::span<const char> bytes);
    void do_control(uint8_t control);
    void do_csi(char command);
    void do_escape(char command);
    void string_fragment(std::span<const char> str, bool final_);
    [[nodiscard]] bool is_string_state() const;

    // Keyboard
    void keyboard_unichar(uint32_t c, Modifier mod);
    void keyboard_key(Key key, Modifier mod);
    void keyboard_start_paste();
    void keyboard_end_paste();
    void emit_key_literal(char literal, int32_t imod);
    void emit_key_ss3(char literal, int32_t imod);
    void emit_key_csi(char literal, int32_t imod);

    // Mouse
    void mouse_move(int32_t row, int32_t col, Modifier mod);
    void mouse_button(int32_t button, bool pressed, Modifier mod);

    // Init
    State::Impl& obtain_state();
    Screen::Impl& obtain_screen();
};

// --- Unicode width / combining ---

struct UnicodeInterval {
    uint32_t first = 0;
    uint32_t last = 0;
};

[[nodiscard]] constexpr bool unicode_bisearch(uint32_t ucs, std::span<const UnicodeInterval> table) {
    if(table.empty())
        return false;

    int32_t min = 0;
    int32_t max = static_cast<int32_t>(table.size()) - 1;

    if(ucs < table[0].first || ucs > table[max].last)
        return false;
    while(max >= min) {
        int32_t mid = (min + max) / 2;
        if(ucs > table[mid].last)
            min = mid + 1;
        else if(ucs < table[mid].first)
            max = mid - 1;
        else
            return true;
    }

    return false;
}

// sorted list of non-overlapping intervals of non-spacing characters
// generated by "uniset +cat=Me +cat=Mn +cat=Cf -00AD +1160-11FF +200B c"
inline constexpr auto unicode_combining = std::to_array<UnicodeInterval>({
    { 0x0300, 0x036F }, { 0x0483, 0x0486 }, { 0x0488, 0x0489 },
    { 0x0591, 0x05BD }, { 0x05BF, 0x05BF }, { 0x05C1, 0x05C2 },
    { 0x05C4, 0x05C5 }, { 0x05C7, 0x05C7 }, { 0x0600, 0x0603 },
    { 0x0610, 0x0615 }, { 0x064B, 0x065E }, { 0x0670, 0x0670 },
    { 0x06D6, 0x06E4 }, { 0x06E7, 0x06E8 }, { 0x06EA, 0x06ED },
    { 0x070F, 0x070F }, { 0x0711, 0x0711 }, { 0x0730, 0x074A },
    { 0x07A6, 0x07B0 }, { 0x07EB, 0x07F3 }, { 0x0901, 0x0902 },
    { 0x093C, 0x093C }, { 0x0941, 0x0948 }, { 0x094D, 0x094D },
    { 0x0951, 0x0954 }, { 0x0962, 0x0963 }, { 0x0981, 0x0981 },
    { 0x09BC, 0x09BC }, { 0x09C1, 0x09C4 }, { 0x09CD, 0x09CD },
    { 0x09E2, 0x09E3 }, { 0x0A01, 0x0A02 }, { 0x0A3C, 0x0A3C },
    { 0x0A41, 0x0A42 }, { 0x0A47, 0x0A48 }, { 0x0A4B, 0x0A4D },
    { 0x0A70, 0x0A71 }, { 0x0A81, 0x0A82 }, { 0x0ABC, 0x0ABC },
    { 0x0AC1, 0x0AC5 }, { 0x0AC7, 0x0AC8 }, { 0x0ACD, 0x0ACD },
    { 0x0AE2, 0x0AE3 }, { 0x0B01, 0x0B01 }, { 0x0B3C, 0x0B3C },
    { 0x0B3F, 0x0B3F }, { 0x0B41, 0x0B43 }, { 0x0B4D, 0x0B4D },
    { 0x0B56, 0x0B56 }, { 0x0B82, 0x0B82 }, { 0x0BC0, 0x0BC0 },
    { 0x0BCD, 0x0BCD }, { 0x0C3E, 0x0C40 }, { 0x0C46, 0x0C48 },
    { 0x0C4A, 0x0C4D }, { 0x0C55, 0x0C56 }, { 0x0CBC, 0x0CBC },
    { 0x0CBF, 0x0CBF }, { 0x0CC6, 0x0CC6 }, { 0x0CCC, 0x0CCD },
    { 0x0CE2, 0x0CE3 }, { 0x0D41, 0x0D43 }, { 0x0D4D, 0x0D4D },
    { 0x0DCA, 0x0DCA }, { 0x0DD2, 0x0DD4 }, { 0x0DD6, 0x0DD6 },
    { 0x0E31, 0x0E31 }, { 0x0E34, 0x0E3A }, { 0x0E47, 0x0E4E },
    { 0x0EB1, 0x0EB1 }, { 0x0EB4, 0x0EB9 }, { 0x0EBB, 0x0EBC },
    { 0x0EC8, 0x0ECD }, { 0x0F18, 0x0F19 }, { 0x0F35, 0x0F35 },
    { 0x0F37, 0x0F37 }, { 0x0F39, 0x0F39 }, { 0x0F71, 0x0F7E },
    { 0x0F80, 0x0F84 }, { 0x0F86, 0x0F87 }, { 0x0F90, 0x0F97 },
    { 0x0F99, 0x0FBC }, { 0x0FC6, 0x0FC6 }, { 0x102D, 0x1030 },
    { 0x1032, 0x1032 }, { 0x1036, 0x1037 }, { 0x1039, 0x1039 },
    { 0x1058, 0x1059 }, { 0x1160, 0x11FF }, { 0x135F, 0x135F },
    { 0x1712, 0x1714 }, { 0x1732, 0x1734 }, { 0x1752, 0x1753 },
    { 0x1772, 0x1773 }, { 0x17B4, 0x17B5 }, { 0x17B7, 0x17BD },
    { 0x17C6, 0x17C6 }, { 0x17C9, 0x17D3 }, { 0x17DD, 0x17DD },
    { 0x180B, 0x180D }, { 0x18A9, 0x18A9 }, { 0x1920, 0x1922 },
    { 0x1927, 0x1928 }, { 0x1932, 0x1932 }, { 0x1939, 0x193B },
    { 0x1A17, 0x1A18 }, { 0x1B00, 0x1B03 }, { 0x1B34, 0x1B34 },
    { 0x1B36, 0x1B3A }, { 0x1B3C, 0x1B3C }, { 0x1B42, 0x1B42 },
    { 0x1B6B, 0x1B73 }, { 0x1DC0, 0x1DCA }, { 0x1DFE, 0x1DFF },
    { 0x200B, 0x200F }, { 0x202A, 0x202E }, { 0x2060, 0x2063 },
    { 0x206A, 0x206F }, { 0x20D0, 0x20EF }, { 0x302A, 0x302F },
    { 0x3099, 0x309A }, { 0xA806, 0xA806 }, { 0xA80B, 0xA80B },
    { 0xA825, 0xA826 }, { 0xFB1E, 0xFB1E }, { 0xFE00, 0xFE0F },
    { 0xFE20, 0xFE23 }, { 0xFEFF, 0xFEFF }, { 0xFFF9, 0xFFFB },
    { 0x10A01, 0x10A03 }, { 0x10A05, 0x10A06 }, { 0x10A0C, 0x10A0F },
    { 0x10A38, 0x10A3A }, { 0x10A3F, 0x10A3F }, { 0x1D167, 0x1D169 },
    { 0x1D173, 0x1D182 }, { 0x1D185, 0x1D18B }, { 0x1D1AA, 0x1D1AD },
    { 0x1D242, 0x1D244 }, { 0xE0001, 0xE0001 }, { 0xE0020, 0xE007F },
    { 0xE0100, 0xE01EF },
});

inline constexpr auto unicode_fullwidth = std::to_array<UnicodeInterval>({
#include "fullwidth.inc"
});

[[nodiscard]] constexpr int32_t unicode_mk_wcwidth(uint32_t ucs) {
    if(ucs == 0)
        return 0;
    if(ucs < c0_end || (ucs >= ctrl_del && ucs < c1_end))
        return -1;

    if(unicode_bisearch(ucs, unicode_combining))
        return 0;

    return 1 +
        (ucs >= 0x1100 &&
         (ucs <= 0x115f ||
          ucs == 0x2329 || ucs == 0x232a ||
          (ucs >= 0x2e80 && ucs <= 0xa4cf &&
           ucs != 0x303f) ||
          (ucs >= 0xac00 && ucs <= 0xd7a3) ||
          (ucs >= 0xf900 && ucs <= 0xfaff) ||
          (ucs >= 0xfe10 && ucs <= 0xfe19) ||
          (ucs >= 0xfe30 && ucs <= 0xfe6f) ||
          (ucs >= 0xff00 && ucs <= 0xff60) ||
          (ucs >= 0xffe0 && ucs <= 0xffe6) ||
          (ucs >= 0x2'0000 && ucs <= 0x2'FFFD) ||
          (ucs >= 0x3'0000 && ucs <= 0x3'FFFD)));
}

[[nodiscard]] constexpr int32_t unicode_width(uint32_t codepoint) {
    if(unicode_bisearch(codepoint, unicode_fullwidth))
        return 2;
    return unicode_mk_wcwidth(codepoint);
}

[[nodiscard]] constexpr bool unicode_is_combining(uint32_t codepoint) {
    return unicode_bisearch(codepoint, unicode_combining);
}

// --- scroll_rect / copy_cells templates ---

template<typename MoveRect, typename EraseRect>
    requires std::invocable<MoveRect, Rect, Rect> &&
             std::invocable<EraseRect, Rect, bool>
void scroll_rect(Rect rect, int32_t downward, int32_t rightward,
    MoveRect&& moverect, EraseRect&& eraserect)
{
    Rect src;
    Rect dest;

    if(std::abs(downward)  >= rect.end_row - rect.start_row ||
       std::abs(rightward) >= rect.end_col - rect.start_col) {
        eraserect(rect, false);
        return;
    }

    if(rightward >= 0) {
        dest.start_col = rect.start_col;
        dest.end_col   = rect.end_col   - rightward;
        src.start_col  = rect.start_col + rightward;
        src.end_col    = rect.end_col;
    }
    else {
        int32_t leftward = -rightward;
        dest.start_col = rect.start_col + leftward;
        dest.end_col   = rect.end_col;
        src.start_col  = rect.start_col;
        src.end_col    = rect.end_col - leftward;
    }

    if(downward >= 0) {
        dest.start_row = rect.start_row;
        dest.end_row   = rect.end_row   - downward;
        src.start_row  = rect.start_row + downward;
        src.end_row    = rect.end_row;
    }
    else {
        int32_t upward = -downward;
        dest.start_row = rect.start_row + upward;
        dest.end_row   = rect.end_row;
        src.start_row  = rect.start_row;
        src.end_row    = rect.end_row - upward;
    }

    moverect(dest, src);

    if(downward > 0)
        rect.start_row = rect.end_row - downward;
    else if(downward < 0)
        rect.end_row = rect.start_row - downward;

    if(rightward > 0)
        rect.start_col = rect.end_col - rightward;
    else if(rightward < 0)
        rect.end_col = rect.start_col - rightward;

    eraserect(rect, false);
}

template<typename CopyCell>
    requires std::invocable<CopyCell, Pos, Pos>
void copy_cells(Rect dest, Rect src, CopyCell&& copycell)
{
    int32_t downward  = src.start_row - dest.start_row;
    int32_t rightward = src.start_col - dest.start_col;

    int32_t init_row, test_row, init_col, test_col;
    int32_t inc_row, inc_col;

    if(downward < 0) {
        init_row = dest.end_row - 1;
        test_row = dest.start_row - 1;
        inc_row = -1;
    }
    else {
        init_row = dest.start_row;
        test_row = dest.end_row;
        inc_row = +1;
    }

    if(rightward < 0) {
        init_col = dest.end_col - 1;
        test_col = dest.start_col - 1;
        inc_col = -1;
    }
    else {
        init_col = dest.start_col;
        test_col = dest.end_col;
        inc_col = +1;
    }

    Pos pos;
    for(pos.row = init_row; pos.row != test_row; pos.row += inc_row) {
        for(pos.col = init_col; pos.col != test_col; pos.col += inc_col) {
            Pos srcpos = { pos.row + downward, pos.col + rightward };
            copycell(pos, srcpos);
        }
    }
}

} // namespace vterm

#endif // VTERM_INTERNAL_H
