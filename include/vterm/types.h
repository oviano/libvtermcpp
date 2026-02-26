#ifndef VTERM_TYPES_H
#define VTERM_TYPES_H

#include <algorithm>
#include <array>
#include <compare>
#include <cstdint>
#include <cstddef>
#include <span>
#include <string_view>
#include <type_traits>
#include <vector>

namespace vterm {

template<typename E>
    requires std::is_enum_v<E>
constexpr auto to_underlying(E e) noexcept -> std::underlying_type_t<E> {
    return static_cast<std::underlying_type_t<E>>(e);
}

inline constexpr int32_t version_major = 0;
inline constexpr int32_t version_minor = 3;
inline constexpr int32_t version_patch = 3;

inline constexpr int32_t max_chars_per_cell = 6;

// --- Modifier / Key enums ---

enum class Modifier : uint8_t {
    None  = 0x00,
    Shift = 0x01,
    Alt   = 0x02,
    Ctrl  = 0x04,
};

inline constexpr uint8_t all_mods_mask = 0x07;

constexpr Modifier operator|(Modifier a, Modifier b) noexcept {
    return static_cast<Modifier>(to_underlying(a) | to_underlying(b));
}
constexpr Modifier operator&(Modifier a, Modifier b) noexcept {
    return static_cast<Modifier>(to_underlying(a) & to_underlying(b));
}
constexpr bool operator!(Modifier m) noexcept {
    return to_underlying(m) == 0;
}
constexpr Modifier operator~(Modifier m) noexcept {
    return static_cast<Modifier>(~to_underlying(m));
}
constexpr Modifier& operator|=(Modifier& a, Modifier b) noexcept { a = a | b; return a; }
constexpr Modifier& operator&=(Modifier& a, Modifier b) noexcept { a = a & b; return a; }

enum class Key : int32_t {
    None = 0,

    Enter,
    Tab,
    Backspace,
    Escape,

    Up,
    Down,
    Left,
    Right,

    Ins,
    Del,
    Home,
    End,
    PageUp,
    PageDown,

    Function0   = 256,
    FunctionMax = Function0 + 255,

    KP0,
    KP1,
    KP2,
    KP3,
    KP4,
    KP5,
    KP6,
    KP7,
    KP8,
    KP9,
    KPMult,
    KPPlus,
    KPComma,
    KPMinus,
    KPPeriod,
    KPDivide,
    KPEnter,
    KPEqual,

    Max,
    NKeys = Max,
};

[[nodiscard]] constexpr Key key_function(int32_t n) { return static_cast<Key>(to_underlying(Key::Function0) + n); }

// --- Position / Rectangle ---

struct Pos {
    int32_t row = 0;
    int32_t col = 0;
    auto operator<=>(const Pos&) const noexcept = default;
};

struct Rect {
    int32_t start_row = 0;
    int32_t end_row = 0;
    int32_t start_col = 0;
    int32_t end_col = 0;

    [[nodiscard]] constexpr bool contains(Pos p) const {
        return p.row >= start_row && p.row < end_row &&
               p.col >= start_col && p.col < end_col;
    }

    [[nodiscard]] constexpr bool contains_rect(const Rect& other) const {
        return other.start_row >= start_row &&
               other.start_col >= start_col &&
               other.end_row   <= end_row   &&
               other.end_col   <= end_col;
    }

    [[nodiscard]] constexpr bool intersects(const Rect& other) const {
        return start_row < other.end_row   &&
               other.start_row < end_row   &&
               start_col < other.end_col   &&
               other.start_col < end_col;
    }

    constexpr void expand(const Rect& other) {
        if(other.start_row < start_row) start_row = other.start_row;
        if(other.start_col < start_col) start_col = other.start_col;
        if(other.end_row   > end_row)   end_row   = other.end_row;
        if(other.end_col   > end_col)   end_col   = other.end_col;
    }

    constexpr void clip(const Rect& bounds) {
        start_row = std::max(start_row, bounds.start_row);
        start_col = std::max(start_col, bounds.start_col);
        end_row   = std::max(start_row, std::min(end_row, bounds.end_row));
        end_col   = std::max(start_col, std::min(end_col, bounds.end_col));
    }

    constexpr void move(int32_t row_delta, int32_t col_delta) {
        start_row += row_delta; end_row += row_delta;
        start_col += col_delta; end_col += col_delta;
    }

    constexpr bool operator==(const Rect& other) const noexcept = default;
};

// --- Color ---

namespace color_type {
    inline constexpr uint8_t rgb          = 0x00;
    inline constexpr uint8_t indexed      = 0x01;
    inline constexpr uint8_t type_mask    = 0x01;
    inline constexpr uint8_t default_fg   = 0x02;
    inline constexpr uint8_t default_bg   = 0x04;
    inline constexpr uint8_t default_mask = 0x06;
}

union Color {
    uint8_t type = 0;

    struct {
        uint8_t type;
        uint8_t red, green, blue;
    } rgb;

    struct {
        uint8_t type;
        uint8_t idx;
    } indexed;

    [[nodiscard]] constexpr bool is_rgb() const        { return (type & color_type::type_mask) == color_type::rgb; }
    [[nodiscard]] constexpr bool is_indexed() const    { return (type & color_type::type_mask) == color_type::indexed; }
    [[nodiscard]] constexpr bool is_default_fg() const { return (type & color_type::default_fg) != 0; }
    [[nodiscard]] constexpr bool is_default_bg() const { return (type & color_type::default_bg) != 0; }

    static constexpr Color from_rgb(uint8_t r, uint8_t g, uint8_t b) {
        return Color{.rgb = {color_type::rgb, r, g, b}};
    }

    static constexpr Color from_index(uint8_t idx) {
        return Color{.indexed = {color_type::indexed, idx}};
    }

    constexpr bool operator==(const Color& other) const noexcept {
        if(is_indexed() && other.is_indexed())
            return indexed.idx == other.indexed.idx;
        if(is_rgb() && other.is_rgb())
            return rgb.red == other.rgb.red && rgb.green == other.rgb.green && rgb.blue == other.rgb.blue;
        return false;
    }
};

// --- Value types ---

enum class ValueType {
    None = 0,
    Bool = 1,
    Int,
    String,
    Color,

    NValueTypes,
};

struct StringFragment {
    std::string_view str;
    bool initial = false;
    bool final_  = false;
};

union Value {
    bool    boolean;
    int32_t number;
    StringFragment string;
    vterm::Color color;

    Value() : number(0) {}
};

// --- Attr / Prop enums ---

enum class Attr {
    Bold = 1,
    Underline,
    Italic,
    Blink,
    Reverse,
    Conceal,
    Strike,
    Font,
    Foreground,
    Background,
    Small,
    Baseline,

    NAttrs,
};

enum class Prop {
    CursorVisible = 1,
    CursorBlink,
    AltScreen,
    Title,
    IconName,
    Reverse,
    CursorShape,
    Mouse,
    FocusReport,

    NProps,
};

enum class CursorShape : int32_t {
    Block = 1,
    Underline,
    BarLeft,
};

enum class MouseProp : int32_t {
    None = 0,
    Click,
    Drag,
    Move,
};

enum class SelectionMask : uint16_t {
    Clipboard = (1 << 0),
    Primary   = (1 << 1),
    Secondary = (1 << 2),
    Select    = (1 << 3),
    Cut0      = (1 << 4),
};

constexpr SelectionMask operator|(SelectionMask a, SelectionMask b) noexcept {
    return static_cast<SelectionMask>(to_underlying(a) | to_underlying(b));
}
constexpr SelectionMask operator&(SelectionMask a, SelectionMask b) noexcept {
    return static_cast<SelectionMask>(to_underlying(a) & to_underlying(b));
}
constexpr bool operator!(SelectionMask m) noexcept { return to_underlying(m) == 0; }

// --- Glyph / Line info ---

struct GlyphInfo {
    std::span<const uint32_t> chars;
    int32_t         width = 0;
    uint32_t        protected_cell : 1 = 0;
    uint32_t        dwl : 1 = 0;
    uint32_t        dhl : 2 = 0;
};

struct LineInfo {
    uint32_t doublewidth  : 1 = 0;
    uint32_t doubleheight : 2 = 0;
    uint32_t continuation : 1 = 0;
};

struct StateFields {
    Pos       pos;
    std::array<std::vector<LineInfo>*, 2> lineinfos{};
};

// --- Screen cell ---

enum class Underline : uint32_t {
    Off,
    Single,
    Double,
    Curly,
};

enum class Baseline : uint32_t {
    Normal,
    Raise,
    Lower,
};

struct CellAttrs {
    uint32_t  bold      : 1 = 0;
    Underline underline : 2 = Underline::Off;
    uint32_t  italic    : 1 = 0;
    uint32_t  blink     : 1 = 0;
    uint32_t  reverse   : 1 = 0;
    uint32_t  conceal   : 1 = 0;
    uint32_t  strike    : 1 = 0;
    uint32_t  font      : 4 = 0;
    uint32_t  dwl       : 1 = 0;
    uint32_t  dhl       : 2 = 0;
    uint32_t  small     : 1 = 0;
    Baseline  baseline  : 2 = Baseline::Normal;
};

struct ScreenCell {
    std::array<uint32_t, max_chars_per_cell> chars{};
    int8_t    width = 0;
    CellAttrs attrs{};
    Color     fg{}, bg{};
};

// --- Damage ---

enum class DamageSize {
    Cell,
    Row,
    Screen,
    Scroll,

    NDamages,
};

// --- Attr mask (for get_attrs_extent) ---

enum class AttrMask : uint32_t {
    Bold       = 1 << 0,
    Underline  = 1 << 1,
    Italic     = 1 << 2,
    Blink      = 1 << 3,
    Reverse    = 1 << 4,
    Strike     = 1 << 5,
    Font       = 1 << 6,
    Foreground = 1 << 7,
    Background = 1 << 8,
    Conceal    = 1 << 9,
    Small      = 1 << 10,
    Baseline   = 1 << 11,

    All        = (1 << 12) - 1,
};

constexpr AttrMask operator|(AttrMask a, AttrMask b) noexcept {
    return static_cast<AttrMask>(to_underlying(a) | to_underlying(b));
}
constexpr AttrMask operator&(AttrMask a, AttrMask b) noexcept {
    return static_cast<AttrMask>(to_underlying(a) & to_underlying(b));
}
constexpr bool operator!(AttrMask m) noexcept { return to_underlying(m) == 0; }

// --- CSI arg helpers ---

inline constexpr int64_t csi_arg_flag_more = (1LL << 31);
inline constexpr int64_t csi_arg_mask      = (1LL << 31) - 1;
inline constexpr int64_t csi_arg_missing   = (1LL << 31) - 1;

[[nodiscard]] constexpr bool csi_arg_has_more(int64_t a)   { return (a & csi_arg_flag_more) != 0; }
[[nodiscard]] constexpr int64_t csi_arg(int64_t a)            { return a & csi_arg_mask; }
[[nodiscard]] constexpr int32_t csi_arg_i32(int64_t a)        { return static_cast<int32_t>(csi_arg(a)); }
[[nodiscard]] constexpr bool csi_arg_is_missing(int64_t a) { return csi_arg(a) == csi_arg_missing; }
[[nodiscard]] constexpr int64_t csi_arg_or(int64_t a, int64_t def) { return csi_arg_is_missing(a) ? def : csi_arg(a); }
[[nodiscard]] constexpr int64_t csi_arg_count(int64_t a)      { return (csi_arg_is_missing(a) || csi_arg(a) == 0) ? 1 : csi_arg(a); }

// --- Utility functions ---

inline constexpr std::array<ValueType, static_cast<size_t>(Attr::NAttrs)> attr_type_table = {{
    ValueType::None,  // 0 (unused, enums start at 1)
    ValueType::Bool,  // Bold
    ValueType::Int,   // Underline
    ValueType::Bool,  // Italic
    ValueType::Bool,  // Blink
    ValueType::Bool,  // Reverse
    ValueType::Bool,  // Conceal
    ValueType::Bool,  // Strike
    ValueType::Int,   // Font
    ValueType::Color, // Foreground
    ValueType::Color, // Background
    ValueType::Bool,  // Small
    ValueType::Int,   // Baseline
}};

static_assert(attr_type_table.size() == static_cast<size_t>(Attr::NAttrs),
    "attr_type_table must have one entry per Attr value");

[[nodiscard]] constexpr ValueType get_attr_type(Attr attr) {
    auto i = static_cast<size_t>(to_underlying(attr));
    return i < attr_type_table.size() ? attr_type_table[i] : ValueType::None;
}

inline constexpr std::array<ValueType, static_cast<size_t>(Prop::NProps)> prop_type_table = {{
    ValueType::None,   // 0 (unused, enums start at 1)
    ValueType::Bool,   // CursorVisible
    ValueType::Bool,   // CursorBlink
    ValueType::Bool,   // AltScreen
    ValueType::String, // Title
    ValueType::String, // IconName
    ValueType::Bool,   // Reverse
    ValueType::Int,    // CursorShape
    ValueType::Int,    // Mouse
    ValueType::Bool,   // FocusReport
}};

static_assert(prop_type_table.size() == static_cast<size_t>(Prop::NProps),
    "prop_type_table must have one entry per Prop value");

[[nodiscard]] constexpr ValueType get_prop_type(Prop prop) {
    auto i = static_cast<size_t>(to_underlying(prop));
    return i < prop_type_table.size() ? prop_type_table[i] : ValueType::None;
}

} // namespace vterm

#endif // VTERM_TYPES_H
