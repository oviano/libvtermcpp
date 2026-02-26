#include "internal.h"

namespace vterm {

namespace {

constexpr uint8_t default_fg_grey = 240;

constexpr std::array<Color, palette_ansi_count> ansi_colors = {{
    Color::from_rgb(  0,   0,   0), // black
    Color::from_rgb(224,   0,   0), // red
    Color::from_rgb(  0, 224,   0), // green
    Color::from_rgb(224, 224,   0), // yellow
    Color::from_rgb(  0,   0, 224), // blue
    Color::from_rgb(224,   0, 224), // magenta
    Color::from_rgb(  0, 224, 224), // cyan
    Color::from_rgb(224, 224, 224), // white == light grey

    // high intensity
    Color::from_rgb(128, 128, 128), // black
    Color::from_rgb(255,  64,  64), // red
    Color::from_rgb( 64, 255,  64), // green
    Color::from_rgb(255, 255,  64), // yellow
    Color::from_rgb( 64,  64, 255), // blue
    Color::from_rgb(255,  64, 255), // magenta
    Color::from_rgb( 64, 255, 255), // cyan
    Color::from_rgb(255, 255, 255), // white for real
}};

constexpr void lookup_default_colour_ansi(int64_t idx, Color& col) {
    if(idx >= 0 && idx < palette_ansi_count)
        col = ansi_colors[idx];
}

[[nodiscard]] constexpr int32_t getpen_color(const Color& col, int32_t argi, std::span<int64_t> args, bool is_fg) {
    if(( is_fg && col.is_default_fg()) ||
       (!is_fg && col.is_default_bg()))
        return argi;

    if(col.is_indexed()) {
        uint8_t idx = col.indexed.idx;
        if(idx < palette_normal_count) {
            args[argi++] = (idx + (is_fg ? 30 : 40));
        }
        else if(idx < palette_ansi_count) {
            args[argi++] = (idx - palette_normal_count + (is_fg ? 90 : 100));
        }
        else {
            args[argi++] = csi_arg_flag_more | (is_fg ? 38 : 48);
            args[argi++] = csi_arg_flag_more | 5;
            args[argi++] = idx;
        }
    }
    else if(col.is_rgb()) {
        args[argi++] = csi_arg_flag_more | (is_fg ? 38 : 48);
        args[argi++] = csi_arg_flag_more | 2;
        args[argi++] = csi_arg_flag_more | col.rgb.red;
        args[argi++] = csi_arg_flag_more | col.rgb.green;
        args[argi++] = col.rgb.blue;
    }
    return argi;
}

} // anonymous namespace

bool State::Impl::lookup_colour_ansi(int64_t index, Color& col) const {
    if(index >= 0 && index < palette_ansi_count) {
        col = colors[index];
        return true;
    }
    return false;
}

bool State::Impl::lookup_colour_palette(int64_t index, Color& col) const {
    if(index >= 0 && index < palette_ansi_count) {
        return lookup_colour_ansi(index, col);
    }
    else if(index >= palette_ansi_count && index < palette_cube_end) {
        index -= palette_ansi_count;
        col = Color::from_rgb(ramp6[index/6/6 % 6], ramp6[index/6 % 6], ramp6[index % 6]);
        return true;
    }
    else if(index >= palette_cube_end && index < palette_max) {
        index -= palette_cube_end;
        col = Color::from_rgb(ramp24[index], ramp24[index], ramp24[index]);
        return true;
    }
    return false;
}

int32_t State::Impl::lookup_colour(int32_t palette, std::span<const int64_t> args, Color& col) const {
    int32_t argcount = static_cast<int32_t>(args.size());
    switch(palette) {
    case 2: // RGB mode
        if(argcount < 3)
            return argcount;
        col = Color::from_rgb(
            static_cast<uint8_t>(csi_arg(args[0])),
            static_cast<uint8_t>(csi_arg(args[1])),
            static_cast<uint8_t>(csi_arg(args[2])));
        return 3;

    case 5: // XTerm 256-colour mode
        if(!argcount || csi_arg_is_missing(args[0]))
            return argcount ? 1 : 0;
        col = Color::from_index(static_cast<uint8_t>(args[0]));
        return argcount ? 1 : 0;

    default:
        DEBUG_LOG("Unrecognised colour palette {}\n", palette);
        return 0;
    }
}

void State::Impl::setpenattr(Attr attr, ValueType type, const Value& val) {
#ifdef DEBUG
    if(type != get_attr_type(attr)) {
        DEBUG_LOG("Cannot set attr {} as it has type {}, not type {}\n",
            to_underlying(attr), to_underlying(get_attr_type(attr)), to_underlying(type));
        return;
    }
#endif
    if(callbacks)
        callbacks->on_setpenattr(attr, val);
}

void State::Impl::setpenattr_bool(Attr attr, bool boolean) {
    Value val{};
    val.boolean = boolean;
    setpenattr(attr, ValueType::Bool, val);
}

void State::Impl::setpenattr_int(Attr attr, int32_t number) {
    Value val{};
    val.number = number;
    setpenattr(attr, ValueType::Int, val);
}

void State::Impl::setpenattr_col(Attr attr, Color color) {
    Value val{};
    val.color = color;
    setpenattr(attr, ValueType::Color, val);
}

void State::Impl::set_pen_col_ansi(Attr attr, int64_t col) {
    Color& colref = (attr == Attr::Background) ? pen.bg : pen.fg;
    colref = Color::from_index(static_cast<uint8_t>(col));
    setpenattr_col(attr, colref);
}

void State::Impl::newpen() {
    default_fg = Color::from_rgb(default_fg_grey, default_fg_grey, default_fg_grey);
    default_bg = Color::from_rgb(0, 0, 0);

    // Set default color flags
    default_fg.type = (default_fg.type & ~color_type::default_mask) | color_type::default_fg;
    default_bg.type = (default_bg.type & ~color_type::default_mask) | color_type::default_bg;

    for(int32_t col = 0; col < palette_ansi_count; col++)
        lookup_default_colour_ansi(col, colors[col]);
}

void State::Impl::resetpen() {
    pen.bold = false;      setpenattr_bool(Attr::Bold, false);
    pen.underline = Underline::Off; setpenattr_int(Attr::Underline, to_underlying(pen.underline));
    pen.italic = false;    setpenattr_bool(Attr::Italic, false);
    pen.blink = false;     setpenattr_bool(Attr::Blink, false);
    pen.reverse = false;   setpenattr_bool(Attr::Reverse, false);
    pen.conceal = false;   setpenattr_bool(Attr::Conceal, false);
    pen.strike = false;    setpenattr_bool(Attr::Strike, false);
    pen.font = 0;          setpenattr_int (Attr::Font, 0);
    pen.small = false;     setpenattr_bool(Attr::Small, false);
    pen.baseline = Baseline::Normal; setpenattr_int(Attr::Baseline, to_underlying(pen.baseline));

    pen.fg = default_fg;  setpenattr_col(Attr::Foreground, default_fg);
    pen.bg = default_bg;  setpenattr_col(Attr::Background, default_bg);
}

void State::Impl::savepen(bool save) {
    if(save) {
        saved.pen = pen;
    }
    else {
        pen = saved.pen;

        setpenattr_bool(Attr::Bold,      pen.bold);
        setpenattr_int (Attr::Underline, to_underlying(pen.underline));
        setpenattr_bool(Attr::Italic,    pen.italic);
        setpenattr_bool(Attr::Blink,     pen.blink);
        setpenattr_bool(Attr::Reverse,   pen.reverse);
        setpenattr_bool(Attr::Conceal,   pen.conceal);
        setpenattr_bool(Attr::Strike,    pen.strike);
        setpenattr_int (Attr::Font,      pen.font);
        setpenattr_bool(Attr::Small,     pen.small);
        setpenattr_int (Attr::Baseline,  to_underlying(pen.baseline));

        setpenattr_col(Attr::Foreground, pen.fg);
        setpenattr_col(Attr::Background, pen.bg);
    }
}

void State::Impl::setpen(std::span<const int64_t> args) {
    int32_t argcount = static_cast<int32_t>(args.size());
    int32_t argi = 0;

    while(argi < argcount) {
        switch(int64_t arg = csi_arg(args[argi])) {
        case csi_arg_missing:
        case 0: // Reset
            resetpen();
            break;

        case 1: { // Bold on
            const Color& fg = pen.fg;
            pen.bold = true;
            setpenattr_bool(Attr::Bold, true);
            if(!fg.is_default_fg() && fg.is_indexed() && fg.indexed.idx < palette_normal_count && bold_is_highbright)
                set_pen_col_ansi(Attr::Foreground, fg.indexed.idx + (pen.bold ? palette_normal_count : 0));
            break;
        }

        case 3:
            pen.italic = true;
            setpenattr_bool(Attr::Italic, true);
            break;

        case 4: // Underline
            pen.underline = Underline::Single;
            if(csi_arg_has_more(args[argi]) && argi + 1 < argcount) {
                argi++;
                switch(csi_arg(args[argi])) {
                case 0: pen.underline = Underline::Off; break;
                case 1: pen.underline = Underline::Single; break;
                case 2: pen.underline = Underline::Double; break;
                case 3: pen.underline = Underline::Curly; break;
                }
            }
            setpenattr_int(Attr::Underline, to_underlying(pen.underline));
            break;

        case 5:
            pen.blink = true;
            setpenattr_bool(Attr::Blink, true);
            break;

        case 7:
            pen.reverse = true;
            setpenattr_bool(Attr::Reverse, true);
            break;

        case 8:
            pen.conceal = true;
            setpenattr_bool(Attr::Conceal, true);
            break;

        case 9:
            pen.strike = true;
            setpenattr_bool(Attr::Strike, true);
            break;

        case 10: case 11: case 12: case 13: case 14:
        case 15: case 16: case 17: case 18: case 19:
            pen.font = static_cast<int32_t>(arg - 10);
            setpenattr_int(Attr::Font, pen.font);
            break;

        case 21:
            pen.underline = Underline::Double;
            setpenattr_int(Attr::Underline, to_underlying(pen.underline));
            break;

        case 22:
            pen.bold = false;
            setpenattr_bool(Attr::Bold, false);
            break;

        case 23:
            pen.italic = false;
            setpenattr_bool(Attr::Italic, false);
            break;

        case 24:
            pen.underline = Underline::Off;
            setpenattr_int(Attr::Underline, to_underlying(pen.underline));
            break;

        case 25:
            pen.blink = false;
            setpenattr_bool(Attr::Blink, false);
            break;

        case 27:
            pen.reverse = false;
            setpenattr_bool(Attr::Reverse, false);
            break;

        case 28:
            pen.conceal = false;
            setpenattr_bool(Attr::Conceal, false);
            break;

        case 29:
            pen.strike = false;
            setpenattr_bool(Attr::Strike, false);
            break;

        case 30: case 31: case 32: case 33:
        case 34: case 35: case 36: case 37:
        case 40: case 41: case 42: case 43:
        case 44: case 45: case 46: case 47: {
            bool is_bg = (arg >= 40);
            int64_t idx = arg - (is_bg ? 40 : 30);
            if(!is_bg && pen.bold && bold_is_highbright)
                idx += palette_normal_count;
            set_pen_col_ansi(is_bg ? Attr::Background : Attr::Foreground, idx);
            break;
        }

        case 38:
        case 48: {
            if(argcount - argi < 2)
                return;
            bool is_bg = (arg == 48);
            Color& colref = is_bg ? pen.bg : pen.fg;
            Attr attr = is_bg ? Attr::Background : Attr::Foreground;
            argi += 1 + lookup_colour(csi_arg(args[argi+1]), args.subspan(argi+2), colref);
            setpenattr_col(attr, colref);
            break;
        }

        case 39:
        case 49: {
            bool is_bg = (arg == 49);
            Color& colref = is_bg ? pen.bg : pen.fg;
            colref = is_bg ? default_bg : default_fg;
            setpenattr_col(is_bg ? Attr::Background : Attr::Foreground, colref);
            break;
        }

        case 73: case 74: case 75:
            pen.small = (arg != 75);
            pen.baseline =
                (arg == 73) ? Baseline::Raise :
                (arg == 74) ? Baseline::Lower :
                              Baseline::Normal;
            setpenattr_bool(Attr::Small,    pen.small);
            setpenattr_int (Attr::Baseline, to_underlying(pen.baseline));
            break;

        case 90: case 91: case 92: case 93:
        case 94: case 95: case 96: case 97:
        case 100: case 101: case 102: case 103:
        case 104: case 105: case 106: case 107: {
            bool is_bg = (arg >= 100);
            set_pen_col_ansi(is_bg ? Attr::Background : Attr::Foreground,
                arg - (is_bg ? 100 : 90) + palette_normal_count);
            break;
        }

        default:
            DEBUG_LOG("libvterm: Unhandled CSI SGR {}\n", arg);
            break;
        }

        while(argi < argcount && csi_arg_has_more(args[argi]))
            argi++;
        if(argi < argcount)
            argi++;
    }
}

int32_t State::Impl::getpen(std::span<int64_t> args) {
    int32_t argi = 0;

    if(pen.bold)      args[argi++] = 1;
    if(pen.italic)    args[argi++] = 3;
    if(pen.underline == Underline::Single)
        args[argi++] = 4;
    if(pen.underline == Underline::Curly) {
        args[argi++] = 4 | csi_arg_flag_more;
        args[argi++] = 3;
    }
    if(pen.blink)     args[argi++] = 5;
    if(pen.reverse)   args[argi++] = 7;
    if(pen.conceal)   args[argi++] = 8;
    if(pen.strike)    args[argi++] = 9;
    if(pen.font)      args[argi++] = 10 + pen.font;
    if(pen.underline == Underline::Double)
        args[argi++] = 21;

    argi = getpen_color(pen.fg, argi, args, true);
    argi = getpen_color(pen.bg, argi, args, false);

    if(pen.small) {
        if(pen.baseline == Baseline::Raise)
            args[argi++] = 73;
        else if(pen.baseline == Baseline::Lower)
            args[argi++] = 74;
    }

    return argi;
}

// --- State public API (pen-related) ---

void State::set_default_colors(const Color& fg, const Color& bg) {
    impl_->default_fg = fg;
    impl_->default_fg.type = (impl_->default_fg.type & ~color_type::default_mask) | color_type::default_fg;
    impl_->default_bg = bg;
    impl_->default_bg.type = (impl_->default_bg.type & ~color_type::default_mask) | color_type::default_bg;
}

State::ColorPair State::get_default_colors() const {
    return {impl_->default_fg, impl_->default_bg};
}

Color State::get_palette_color(int32_t index) const {
    Color col{};
    (void)impl_->lookup_colour_palette(index, col);
    return col;
}

void State::set_palette_color(int32_t index, const Color& col) {
    if(index >= 0 && index < palette_ansi_count)
        impl_->colors[index] = col;
}

void State::convert_color_to_rgb(Color& col) const {
    if(col.is_indexed())
        (void)impl_->lookup_colour_palette(col.indexed.idx, col);
    col.type &= color_type::type_mask;
}

void State::set_bold_highbright(bool enabled) {
    impl_->bold_is_highbright = enabled;
}

bool State::get_penattr(Attr attr, Value& val) const {
    switch(attr) {
    case Attr::Bold:       val.boolean = impl_->pen.bold;      return true;
    case Attr::Underline:  val.number  = to_underlying(impl_->pen.underline); return true;
    case Attr::Italic:     val.boolean = impl_->pen.italic;    return true;
    case Attr::Blink:      val.boolean = impl_->pen.blink;     return true;
    case Attr::Reverse:    val.boolean = impl_->pen.reverse;   return true;
    case Attr::Conceal:    val.boolean = impl_->pen.conceal;   return true;
    case Attr::Strike:     val.boolean = impl_->pen.strike;    return true;
    case Attr::Font:       val.number  = impl_->pen.font;      return true;
    case Attr::Foreground: val.color   = impl_->pen.fg;        return true;
    case Attr::Background: val.color   = impl_->pen.bg;        return true;
    case Attr::Small:      val.boolean = impl_->pen.small;     return true;
    case Attr::Baseline:   val.number  = to_underlying(impl_->pen.baseline);  return true;
    case Attr::NAttrs:     return false;
    }
    return false;
}

} // namespace vterm
