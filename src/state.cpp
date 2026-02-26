#include "internal.h"
#include "utf8.h"

#include <array>
#include <bit>
#include <cstdlib>
#include <string_view>

#if defined(DEBUG) && DEBUG > 1
# define DEBUG_GLYPH_COMBINE
#endif

namespace vterm {
namespace {

constexpr int32_t da2_firmware_version = 100;

} // anonymous namespace

// ---- Convenient wrappers to make callback functions easier ----

void State::Impl::putglyph(std::span<const uint32_t> chars, int32_t width, Pos pos)
{
    GlyphInfo info{};
    info.chars = chars;
    info.width = width;
    info.protected_cell = protected_cell;
    info.dwl = get_lineinfo(pos.row).doublewidth;
    info.dhl = get_lineinfo(pos.row).doubleheight;

    if(callbacks)
        if(callbacks->on_putglyph(info, pos))
            return;

    DEBUG_LOG("libvterm: Unhandled putglyph U+{:04x} at ({},{})\n", chars[0], pos.col, pos.row);
}

void State::Impl::updatecursor(const Pos& oldpos, bool cancel_phantom)
{
    if(pos.col == oldpos.col && pos.row == oldpos.row)
        return;

    if(cancel_phantom)
        at_phantom = false;

    if(callbacks)
        if(callbacks->on_movecursor(pos, oldpos, mode.cursor_visible))
            return;
}

void State::Impl::erase(Rect rect, bool selective)
{
    if(rect.end_col == cols) {
        // If we're erasing the final cells of any lines, cancel the continuation
        // marker on the subsequent line
        for(int32_t row = rect.start_row + 1; row < rect.end_row + 1 && row < rows; row++)
            get_lineinfo(row).continuation = false;
    }

    if(callbacks)
        if(callbacks->on_erase(rect, selective))
            return;
}

void State::Impl::scroll(Rect rect, int32_t downward, int32_t rightward)
{
    if(!downward && !rightward)
        return;

    int32_t rows_ = rect.end_row - rect.start_row;
    if(downward > rows_)
        downward = rows_;
    else if(downward < -rows_)
        downward = -rows_;

    int32_t cols_ = rect.end_col - rect.start_col;
    if(rightward > cols_)
        rightward = cols_;
    else if(rightward < -cols_)
        rightward = -cols_;

    if(callbacks_has_premove && callbacks) {
        // TODO: technically this logic is wrong if both downward != 0 and rightward != 0

        // Work out what subsection of the destination area is about to be destroyed
        if(downward > 0) {
            // about to destroy the top
            Rect r{.start_row = rect.start_row, .end_row = rect.start_row + downward, .start_col = rect.start_col, .end_col = rect.end_col};
            callbacks->on_premove(r);
        }
        else if(downward < 0) {
            // about to destroy the bottom
            Rect r{.start_row = rect.end_row + downward, .end_row = rect.end_row, .start_col = rect.start_col, .end_col = rect.end_col};
            callbacks->on_premove(r);
        }

        if(rightward > 0) {
            // about to destroy the left
            Rect r{.start_row = rect.start_row, .end_row = rect.end_row, .start_col = rect.start_col, .end_col = rect.start_col + rightward};
            callbacks->on_premove(r);
        }
        else if(rightward < 0) {
            // about to destroy the right
            Rect r{.start_row = rect.start_row, .end_row = rect.end_row, .start_col = rect.end_col + rightward, .end_col = rect.end_col};
            callbacks->on_premove(r);
        }
    }

    // Update lineinfo if full line
    if(rect.start_col == 0 && rect.end_col == cols && rightward == 0) {
        int32_t height = rect.end_row - rect.start_row - std::abs(downward);

        if(downward > 0) {
            auto& li = lineinfos[lineinfo_bufidx];
            std::copy(li.begin() + rect.start_row + downward,
                      li.begin() + rect.start_row + downward + height,
                      li.begin() + rect.start_row);
            for(int32_t row = rect.end_row - downward; row < rect.end_row; row++)
                get_lineinfo(row) = LineInfo{};
        }
        else {
            auto& li = lineinfos[lineinfo_bufidx];
            std::copy_backward(li.begin() + rect.start_row,
                               li.begin() + rect.start_row + height,
                               li.begin() + rect.start_row - downward + height);
            for(int32_t row = rect.start_row; row < rect.start_row - downward; row++)
                get_lineinfo(row) = LineInfo{};
        }
    }

    if(callbacks)
        if(callbacks->on_scrollrect(rect, downward, rightward))
            return;

    // Fallback: decompose scroll into moverect + erase via lambdas
    scroll_rect(rect, downward, rightward,
        [this](Rect dest, Rect src) -> bool {
            if(callbacks)
                return callbacks->on_moverect(dest, src);
            return false;
        },
        [this](Rect r, bool selective) -> bool {
            if(callbacks)
                return callbacks->on_erase(r, selective);
            return false;
        });
}

void State::Impl::linefeed()
{
    if(pos.row == scrollregion_bottom_val() - 1) {
        Rect rect{
            .start_row = scrollregion_top,
            .end_row   = scrollregion_bottom_val(),
            .start_col = scrollregion_left_val(),
            .end_col   = scrollregion_right_val(),
        };

        scroll(rect, 1, 0);
    }
    else if(pos.row < rows-1)
        pos.row++;
}

void State::Impl::grow_combine_buffer()
{
    combine_chars.resize(combine_chars.size() * 2);
}

void State::Impl::set_col_tabstop(int32_t col)
{
    uint8_t mask = 1 << (col & tabstop_bit_mask);
    tabstops[col >> tabstop_byte_shift] |= mask;
}

void State::Impl::clear_col_tabstop(int32_t col)
{
    uint8_t mask = 1 << (col & tabstop_bit_mask);
    tabstops[col >> tabstop_byte_shift] &= ~mask;
}

bool State::Impl::is_col_tabstop(int32_t col) const
{
    uint8_t mask = 1 << (col & tabstop_bit_mask);
    return (tabstops[col >> tabstop_byte_shift] & mask) != 0;
}

bool State::Impl::is_cursor_in_scrollregion() const
{
    if(pos.row < scrollregion_top ||
       pos.row >= scrollregion_bottom_val())
        return false;
    if(pos.col < scrollregion_left_val() ||
       pos.col >= scrollregion_right_val())
        return false;

    return true;
}

void State::Impl::tab(int32_t count, int32_t direction)
{
    while(count > 0) {
        if(direction > 0) {
            if(pos.col >= this_row_width()-1)
                return;

            pos.col++;
        }
        else if(direction < 0) {
            if(pos.col < 1)
                return;

            pos.col--;
        }

        if(is_col_tabstop(pos.col))
            count--;
    }
}

void State::Impl::set_lineinfo(int32_t row, bool force, DWL dwl, DHL dhl)
{
    LineInfo info = get_lineinfo(row);

    info.doublewidth  = to_underlying(dwl);
    info.doubleheight = to_underlying(dhl);

    if((callbacks &&
        callbacks->on_setlineinfo(row, info, get_lineinfo(row)))
        || force)
        get_lineinfo(row) = info;
}

// ---- settermprop helpers ----

bool State::Impl::settermprop_bool(Prop prop, bool v)
{
    Value val{};
    val.boolean = v;
    return set_termprop_internal(prop, val);
}

bool State::Impl::settermprop_int(Prop prop, int32_t v)
{
    Value val{};
    val.number = v;
    return set_termprop_internal(prop, val);
}

bool State::Impl::settermprop_string(Prop prop, StringFragment frag)
{
    Value val{};
    val.string = frag;
    return set_termprop_internal(prop, val);
}

void State::Impl::savecursor(bool save)
{
    if(save) {
        saved.pos = pos;
        saved.mode.cursor_visible = mode.cursor_visible;
        saved.mode.cursor_blink   = mode.cursor_blink;
        saved.mode.cursor_shape   = mode.cursor_shape;

        savepen(true);
    }
    else {
        Pos oldpos = pos;

        pos = saved.pos;

        if(pos.row >= rows)
            pos.row = rows - 1;
        if(pos.col >= cols)
            pos.col = cols - 1;

        (void)settermprop_bool(Prop::CursorVisible, saved.mode.cursor_visible);
        (void)settermprop_bool(Prop::CursorBlink,   saved.mode.cursor_blink);
        (void)settermprop_int (Prop::CursorShape,   saved.mode.cursor_shape);

        savepen(false);

        updatecursor(oldpos, true);
    }
}

// ---- Text handler ----

int32_t State::Impl::on_text(std::span<const char> bytes)
{
    Pos oldpos = pos;

    static constexpr int32_t max_decode_codepoints = 1024;
    std::array<uint32_t, max_decode_codepoints> codepoints;

    int32_t npoints = 0;
    size_t eaten = 0;

    EncodingInstance& encoding =
        *(gsingle_set     ? this->encoding[gsingle_set].get() :
          !(bytes[eaten] & high_bit) ? this->encoding[gl_set].get() :
          vt.mode.utf8   ? encoding_utf8.get() :
                                   this->encoding[gr_set].get());

    auto result = encoding.decode(
        std::span{codepoints}.subspan(0, gsingle_set ? 1 : max_decode_codepoints),
        bytes.subspan(eaten));
    npoints += result.codepoints_produced;
    eaten += result.bytes_consumed;

    // There's a chance an encoding (e.g. UTF-8) hasn't found enough bytes yet
    // for even a single codepoint
    if(!npoints)
        return static_cast<int32_t>(eaten);

    if(gsingle_set && npoints)
        gsingle_set = 0;

    int32_t i = 0;

    // This is a combining char. that needs to be merged with the previous
    // glyph output
    if(unicode_is_combining(codepoints[i])) {
        // See if the cursor has moved since
        if(pos.row == combine_pos.row && pos.col == combine_pos.col + combine_width) {
#ifdef DEBUG_GLYPH_COMBINE
            int32_t printpos;
            std::cerr << "DEBUG: COMBINING SPLIT GLYPH of chars {";
            for(printpos = 0; printpos < combine_count; printpos++)
                std::cerr << std::format("U+{:04x} ", combine_chars[printpos]);
            std::cerr << "} + {";
#endif

            // Find where we need to append these combining chars
            int32_t saved_i = combine_count;

            // Add extra ones
            while(i < npoints && unicode_is_combining(codepoints[i])) {
                if(saved_i >= static_cast<int32_t>(combine_chars.size()))
                    grow_combine_buffer();
                combine_chars[saved_i++] = codepoints[i++];
            }
            combine_count = saved_i;

#ifdef DEBUG_GLYPH_COMBINE
            for(; printpos < combine_count; printpos++)
                std::cerr << std::format("U+{:04x} ", combine_chars[printpos]);
            std::cerr << "}\n";
#endif

            // Now render it
            putglyph(make_span(combine_chars.data(), combine_count), combine_width, combine_pos);
        }
        else {
            DEBUG_LOG("libvterm: TODO: Skip over split char+combining\n");
        }
    }

    for(; i < npoints; i++) {
        // Try to find combining characters following this
        int32_t glyph_starts = i;
        int32_t glyph_ends;
        for(glyph_ends = i + 1;
            (glyph_ends < npoints) && (glyph_ends < glyph_starts + max_chars_per_cell);
            glyph_ends++)
            if(!unicode_is_combining(codepoints[glyph_ends]))
                break;

        int32_t width = 0;

        std::array<uint32_t, max_chars_per_cell> chars{};

        for( ; i < glyph_ends; i++) {
            chars[i - glyph_starts] = codepoints[i];
            int32_t this_width = unicode_width(codepoints[i]);
#ifdef DEBUG
            if(this_width < 0) {
                std::cerr << std::format("Text with negative-width codepoint U+{:04x}\n", codepoints[i]);
                std::abort();
            }
#endif
            width += this_width;
        }

        while(i < npoints && unicode_is_combining(codepoints[i]))
            i++;

        int32_t glyph_count = glyph_ends - glyph_starts;
        i--;

#ifdef DEBUG_GLYPH_COMBINE
        int32_t printpos;
        std::cerr << std::format("DEBUG: COMBINED GLYPH of {} chars {{", glyph_ends - glyph_starts);
        for(printpos = 0; printpos < glyph_ends - glyph_starts; printpos++)
            std::cerr << std::format("U+{:04x} ", chars[printpos]);
        std::cerr << std::format("}}, onscreen width {}\n", width);
#endif

        if(at_phantom || pos.col + width > this_row_width()) {
            linefeed();
            pos.col = 0;
            at_phantom = false;
            get_lineinfo(pos.row).continuation = true;
        }

        if(mode.insert) {
            // TODO: This will be a little inefficient for large bodies of text, as
            // it'll have to 'ICH' effectively before every glyph. We should scan
            // ahead and ICH as many times as required
            Rect rect{
                .start_row = pos.row,
                .end_row   = pos.row + 1,
                .start_col = pos.col,
                .end_col   = this_row_width(),
            };
            scroll(rect, 0, -1);
        }

        putglyph(std::span{chars}.subspan(0, glyph_count), width, pos);

        if(i == npoints - 1) {
            // End of the buffer. Save the chars in case we have to combine with
            // more on the next call
            while(glyph_count > static_cast<int32_t>(combine_chars.size()))
                grow_combine_buffer();
            std::copy_n(chars.data(), glyph_count, combine_chars.data());
            combine_count = glyph_count;
            combine_width = width;
            combine_pos = pos;
        }

        if(pos.col + width >= this_row_width()) {
            if(mode.autowrap)
                at_phantom = true;
        }
        else {
            pos.col += width;
        }
    }

    updatecursor(oldpos, false);

#ifdef DEBUG
    if(pos.row < 0 || pos.row >= rows ||
       pos.col < 0 || pos.col >= cols) {
        std::cerr << std::format("Position out of bounds after text: ({},{})\n",
            pos.row, pos.col);
        std::abort();
    }
#endif

    return static_cast<int32_t>(eaten);
}

// ---- Control handler ----

bool State::Impl::on_control(uint8_t control)
{
    Pos oldpos = pos;

    switch(control) {
    case ctrl_bel: // BEL - ECMA-48 8.3.3
        if(callbacks)
            callbacks->on_bell();
        break;

    case ctrl_bs: // BS - ECMA-48 8.3.5
        if(pos.col > 0)
            pos.col--;
        break;

    case ctrl_ht: // HT - ECMA-48 8.3.60
        tab(1, +1);
        break;

    case ctrl_lf: // LF - ECMA-48 8.3.74
    case ctrl_vt: // VT
    case ctrl_ff: // FF
        linefeed();
        if(mode.newline)
            pos.col = 0;
        break;

    case ctrl_cr: // CR - ECMA-48 8.3.15
        pos.col = 0;
        break;

    case ctrl_ls1: // LS1 - ECMA-48 8.3.76
        gl_set = 1;
        break;

    case ctrl_ls0: // LS0 - ECMA-48 8.3.75
        gl_set = 0;
        break;

    case ctrl_ind: // IND - DEPRECATED but implemented for completeness
        linefeed();
        break;

    case ctrl_nel: // NEL - ECMA-48 8.3.86
        linefeed();
        pos.col = 0;
        break;

    case ctrl_hts: // HTS - ECMA-48 8.3.62
        set_col_tabstop(pos.col);
        break;

    case ctrl_ri: // RI - ECMA-48 8.3.104
        if(pos.row == scrollregion_top) {
            Rect rect{
                .start_row = scrollregion_top,
                .end_row   = scrollregion_bottom_val(),
                .start_col = scrollregion_left_val(),
                .end_col   = scrollregion_right_val(),
            };

            scroll(rect, -1, 0);
        }
        else if(pos.row > 0)
            pos.row--;
        break;

    case ctrl_ss2: // SS2 - ECMA-48 8.3.141
        gsingle_set = 2;
        break;

    case ctrl_ss3: // SS3 - ECMA-48 8.3.142
        gsingle_set = 3;
        break;

    default:
        if(fallbacks)
            if(fallbacks->on_control(control))
                return true;

        return false;
    }

    updatecursor(oldpos, true);

#ifdef DEBUG
    if(pos.row < 0 || pos.row >= rows ||
       pos.col < 0 || pos.col >= cols) {
        std::cerr << std::format("Position out of bounds after Ctrl {:02x}: ({},{})\n",
            control, pos.row, pos.col);
        std::abort();
    }
#endif

    return true;
}

// ---- Escape handler ----

bool State::Impl::on_escape(std::string_view bytes)
{
    size_t len = bytes.size();
    // Easier to decode this from the first byte, even though the final
    // byte terminates it
    switch(bytes[0]) {
    case ' ':
        if(len != 2)
            return false;

        switch(bytes[1]) {
            case 'F': // S7C1T
                vt.mode.ctrl8bit = false;
                break;

            case 'G': // S8C1T
                vt.mode.ctrl8bit = true;
                break;

            default:
                return false;
        }
        return true;

    case '#':
        if(len != 2)
            return false;

        switch(bytes[1]) {
            case '3': // DECDHL top
                if(mode.leftrightmargin)
                    break;
                set_lineinfo(pos.row, false, DWL::On, DHL::Top);
                break;

            case '4': // DECDHL bottom
                if(mode.leftrightmargin)
                    break;
                set_lineinfo(pos.row, false, DWL::On, DHL::Bottom);
                break;

            case '5': // DECSWL
                if(mode.leftrightmargin)
                    break;
                set_lineinfo(pos.row, false, DWL::Off, DHL::Off);
                break;

            case '6': // DECDWL
                if(mode.leftrightmargin)
                    break;
                set_lineinfo(pos.row, false, DWL::On, DHL::Off);
                break;

            case '8': // DECALN
            {
                Pos pos;
                std::array<uint32_t, 1> E = { 'E' };
                for(pos.row = 0; pos.row < rows; pos.row++)
                    for(pos.col = 0; pos.col < row_width(pos.row); pos.col++)
                        putglyph(E, 1, pos);
                break;
            }

            default:
                return false;
        }
        return true;

    case '(': case ')': case '*': case '+': // SCS
        if(len != 2)
            return false;

        {
            int32_t setnum = bytes[0] - '(';
            auto newenc = create_encoding(EncodingType::Single94, bytes[1]);

            if(newenc) {
                newenc->init();
                encoding[setnum] = std::move(newenc);
            }
        }

        return true;

    case '7': // DECSC
        savecursor(true);
        return true;

    case '8': // DECRC
        savecursor(false);
        return true;

    case '<': // Ignored by VT100. Used in VT52 mode to switch up to VT100
        return true;

    case '=': // DECKPAM
        mode.keypad = true;
        return true;

    case '>': // DECKPNM
        mode.keypad = false;
        return true;

    case 'c': // RIS - ECMA-48 8.3.105
    {
        Pos oldpos = pos;
        reset(true);
        if(callbacks)
            callbacks->on_movecursor(pos, oldpos, mode.cursor_visible);
        return true;
    }

    case 'n': // LS2 - ECMA-48 8.3.78
        gl_set = 2;
        return true;

    case 'o': // LS3 - ECMA-48 8.3.80
        gl_set = 3;
        return true;

    case '~': // LS1R - ECMA-48 8.3.77
        gr_set = 1;
        return true;

    case '}': // LS2R - ECMA-48 8.3.79
        gr_set = 2;
        return true;

    case '|': // LS3R - ECMA-48 8.3.81
        gr_set = 3;
        return true;

    default:
        return false;
    }
}

// ---- Mode helpers ----

void State::Impl::set_mode(int32_t num, bool val)
{
    switch(num) {
    case 4: // IRM - ECMA-48 7.2.10
        mode.insert = val;
        break;

    case 20: // LNM - ANSI X3.4-1977
        mode.newline = val;
        break;

    default:
        DEBUG_LOG("libvterm: Unknown mode {}\n", num);
        return;
    }
}

void State::Impl::set_dec_mode(int32_t num, bool val)
{
    switch(num) {
    case 1:
        mode.cursor = val;
        break;

    case 5: // DECSCNM - screen mode
        (void)settermprop_bool(Prop::Reverse, val);
        break;

    case 6: // DECOM - origin mode
        {
            Pos oldpos = pos;
            mode.origin = val;
            pos.row = mode.origin ? scrollregion_top : 0;
            pos.col = mode.origin ? scrollregion_left_val() : 0;
            updatecursor(oldpos, true);
        }
        break;

    case 7:
        mode.autowrap = val;
        break;

    case 12:
        (void)settermprop_bool(Prop::CursorBlink, val);
        break;

    case 25:
        (void)settermprop_bool(Prop::CursorVisible, val);
        break;

    case 69: // DECVSSM - vertical split screen mode
             // DECLRMM - left/right margin mode
        mode.leftrightmargin = val;
        if(val) {
            // Setting DECVSSM must clear doublewidth/doubleheight state of every line
            for(int32_t row = 0; row < rows; row++)
                set_lineinfo(row, true, DWL::Off, DHL::Off);
        }

        break;

    case 1000:
    case 1002:
    case 1003:
        (void)settermprop_int(Prop::Mouse,
            !val          ? to_underlying(MouseProp::None)  :
            (num == 1000) ? to_underlying(MouseProp::Click) :
            (num == 1002) ? to_underlying(MouseProp::Drag)  :
                            to_underlying(MouseProp::Move));
        break;

    case 1004:
        (void)settermprop_bool(Prop::FocusReport, val);
        mode.report_focus = val;
        break;

    case 1005:
        mouse_protocol = val ? MouseProtocol::UTF8 : MouseProtocol::X10;
        break;

    case 1006:
        mouse_protocol = val ? MouseProtocol::SGR : MouseProtocol::X10;
        break;

    case 1015:
        mouse_protocol = val ? MouseProtocol::RXVT : MouseProtocol::X10;
        break;

    case 1047:
        (void)settermprop_bool(Prop::AltScreen, val);
        break;

    case 1048:
        savecursor(val);
        break;

    case 1049:
        (void)settermprop_bool(Prop::AltScreen, val);
        savecursor(val);
        break;

    case 2004:
        mode.bracketpaste = val;
        break;

    default:
        DEBUG_LOG("libvterm: Unknown DEC mode {}\n", num);
        return;
    }
}

void State::Impl::request_dec_mode(int32_t num)
{
    int32_t reply;

    switch(num) {
        case 1:
            reply = mode.cursor;
            break;

        case 5:
            reply = mode.screen;
            break;

        case 6:
            reply = mode.origin;
            break;

        case 7:
            reply = mode.autowrap;
            break;

        case 12:
            reply = mode.cursor_blink;
            break;

        case 25:
            reply = mode.cursor_visible;
            break;

        case 69:
            reply = mode.leftrightmargin;
            break;

        case 1000:
            reply = mouse_flags == mouse_want_click;
            break;

        case 1002:
            reply = mouse_flags == (mouse_want_click|mouse_want_drag);
            break;

        case 1003:
            reply = mouse_flags == (mouse_want_click|mouse_want_move);
            break;

        case 1004:
            reply = mode.report_focus;
            break;

        case 1005:
            reply = mouse_protocol == MouseProtocol::UTF8;
            break;

        case 1006:
            reply = mouse_protocol == MouseProtocol::SGR;
            break;

        case 1015:
            reply = mouse_protocol == MouseProtocol::RXVT;
            break;

        case 1047:
            reply = mode.alt_screen;
            break;

        case 2004:
            reply = mode.bracketpaste;
            break;

        default:
            vt.push_output_ctrl( C1::CSI, "?{};{}$y", num, 0);
            return;
    }

    vt.push_output_ctrl( C1::CSI, "?{};{}$y", num, reply ? 1 : 2);
}

void State::Impl::request_version_string()
{
    vt.push_output_str( C1::DCS, true, ">|libvterm({}.{})",
        version_major, version_minor);
}

// ---- CSI handler ----

bool State::Impl::on_csi(std::string_view leader, std::span<const int64_t> args, std::string_view intermed, char command)
{
    int32_t leader_byte = 0;
    int32_t intermed_byte = 0;
    int32_t argcount = static_cast<int32_t>(args.size());
    bool cancel_phantom = true;

    if(!leader.empty()) {
        if(leader.size() > 1) // longer than 1 char
            return false;

        switch(leader[0]) {
        case '?':
        case '>':
            leader_byte = leader[0];
            break;
        default:
            return false;
        }
    }

    if(!intermed.empty()) {
        if(intermed.size() > 1) // longer than 1 char
            return false;

        switch(intermed[0]) {
        case ' ':
        case '!':
        case '"':
        case '$':
        case '\'':
            intermed_byte = intermed[0];
            break;
        default:
            return false;
        }
    }

    Pos oldpos = pos;

    static constexpr auto LEADER = [](int32_t l, int32_t b) constexpr { return (l << 8) | b; };
    static constexpr auto INTERMED = [](int32_t i, int32_t b) constexpr { return (i << 16) | b; };

    switch(intermed_byte << 16 | leader_byte << 8 | command) {
    case '@': { // ICH - ECMA-48 8.3.64
        int32_t count = csi_arg_count(args[0]);

        if(!is_cursor_in_scrollregion())
            break;

        Rect rect{.start_row = pos.row, .end_row = pos.row + 1, .start_col = pos.col,
            .end_col = mode.leftrightmargin ? scrollregion_right_val() : this_row_width()};

        scroll(rect, 0, -count);

        break;
    }

    case 'A': // CUU - ECMA-48 8.3.22
        pos.row -= csi_arg_count(args[0]);
        at_phantom = false;
        break;

    case 'B': // CUD - ECMA-48 8.3.19
        pos.row += csi_arg_count(args[0]);
        at_phantom = false;
        break;

    case 'C': // CUF - ECMA-48 8.3.20
        pos.col += csi_arg_count(args[0]);
        at_phantom = false;
        break;

    case 'D': // CUB - ECMA-48 8.3.18
        pos.col -= csi_arg_count(args[0]);
        at_phantom = false;
        break;

    case 'E': // CNL - ECMA-48 8.3.12
        pos.col = 0;
        pos.row += csi_arg_count(args[0]);
        at_phantom = false;
        break;

    case 'F': // CPL - ECMA-48 8.3.13
        pos.col = 0;
        pos.row -= csi_arg_count(args[0]);
        at_phantom = false;
        break;

    case 'G': // CHA - ECMA-48 8.3.9
        pos.col = csi_arg_or(args[0], 1) - 1;
        at_phantom = false;
        break;

    case 'H': { // CUP - ECMA-48 8.3.21
        int32_t row = csi_arg_or(args[0], 1);
        int32_t col = argcount < 2 || csi_arg_is_missing(args[1]) ? 1 : csi_arg_i32(args[1]);
        // zero-based
        pos.row = row-1;
        pos.col = col-1;
        if(mode.origin) {
            pos.row += scrollregion_top;
            pos.col += scrollregion_left_val();
        }
        at_phantom = false;
        break;
    }

    case 'I': // CHT - ECMA-48 8.3.10
        tab(csi_arg_count(args[0]), +1);
        break;

    case 'J': // ED - ECMA-48 8.3.39
    case LEADER('?', 'J'): { // DECSED - Selective Erase in Display
        bool selective = (leader_byte == '?');
        Rect rect;
        switch(csi_arg(args[0])) {
        case csi_arg_missing:
        case 0:
            rect = {pos.row, pos.row + 1, pos.col, cols};
            if(rect.end_col > rect.start_col)
                erase(rect, selective);

            rect = {pos.row + 1, rows, 0, cols};
            for(int32_t row = rect.start_row; row < rect.end_row; row++)
                set_lineinfo(row, true, DWL::Off, DHL::Off);
            if(rect.end_row > rect.start_row)
                erase(rect, selective);
            break;

        case 1:
            rect = {0, pos.row, 0, cols};
            for(int32_t row = rect.start_row; row < rect.end_row; row++)
                set_lineinfo(row, true, DWL::Off, DHL::Off);
            if(rect.end_col > rect.start_col)
                erase(rect, selective);

            rect = {pos.row, pos.row + 1, 0, pos.col + 1};
            if(rect.end_row > rect.start_row)
                erase(rect, selective);
            break;

        case 2:
            rect = {0, rows, 0, cols};
            for(int32_t row = rect.start_row; row < rect.end_row; row++)
                set_lineinfo(row, true, DWL::Off, DHL::Off);
            erase(rect, selective);
            break;

        case 3:
            if(callbacks)
                if(callbacks->on_sb_clear())
                    return true;
            break;
        }
        break;
    }

    case 'K': // EL - ECMA-48 8.3.41
    case LEADER('?', 'K'): { // DECSEL - Selective Erase in Line
        bool selective = (leader_byte == '?');
        Rect rect;
        rect.start_row = pos.row;
        rect.end_row   = pos.row + 1;

        switch(csi_arg(args[0])) {
        case csi_arg_missing:
        case 0:
            rect.start_col = pos.col; rect.end_col = this_row_width(); break;
        case 1:
            rect.start_col = 0; rect.end_col = pos.col + 1; break;
        case 2:
            rect.start_col = 0; rect.end_col = this_row_width(); break;
        default:
            return false;
        }

        if(rect.end_col > rect.start_col)
            erase(rect, selective);

        break;
    }

    case 'L': { // IL - ECMA-48 8.3.67
        int32_t count = csi_arg_count(args[0]);

        if(!is_cursor_in_scrollregion())
            break;

        Rect rect{.start_row = pos.row, .end_row = scrollregion_bottom_val(), .start_col = scrollregion_left_val(), .end_col = scrollregion_right_val()};
        scroll(rect, -count, 0);

        break;
    }

    case 'M': { // DL - ECMA-48 8.3.32
        int32_t count = csi_arg_count(args[0]);

        if(!is_cursor_in_scrollregion())
            break;

        Rect rect{.start_row = pos.row, .end_row = scrollregion_bottom_val(), .start_col = scrollregion_left_val(), .end_col = scrollregion_right_val()};
        scroll(rect, count, 0);

        break;
    }

    case 'P': { // DCH - ECMA-48 8.3.26
        int32_t count = csi_arg_count(args[0]);

        if(!is_cursor_in_scrollregion())
            break;

        Rect rect{.start_row = pos.row, .end_row = pos.row + 1, .start_col = pos.col,
            .end_col = mode.leftrightmargin ? scrollregion_right_val() : this_row_width()};
        scroll(rect, 0, count);

        break;
    }

    case 'S': { // SU - ECMA-48 8.3.147
        int32_t count = csi_arg_count(args[0]);
        Rect rect{.start_row = scrollregion_top, .end_row = scrollregion_bottom_val(), .start_col = scrollregion_left_val(), .end_col = scrollregion_right_val()};
        scroll(rect, count, 0);
        break;
    }

    case 'T': { // SD - ECMA-48 8.3.113
        int32_t count = csi_arg_count(args[0]);
        Rect rect{.start_row = scrollregion_top, .end_row = scrollregion_bottom_val(), .start_col = scrollregion_left_val(), .end_col = scrollregion_right_val()};
        scroll(rect, -count, 0);
        break;
    }

    case 'X': { // ECH - ECMA-48 8.3.38
        int32_t count = csi_arg_count(args[0]);
        Rect rect{.start_row = pos.row, .end_row = pos.row + 1, .start_col = pos.col, .end_col = std::min(pos.col + count, this_row_width())};
        erase(rect, false);
        break;
    }

    case 'Z': // CBT - ECMA-48 8.3.7
        tab(csi_arg_count(args[0]), -1);
        break;

    case '`': // HPA - ECMA-48 8.3.57
        pos.col = csi_arg_or(args[0], 1) - 1;
        at_phantom = false;
        break;

    case 'a': // HPR - ECMA-48 8.3.59
        pos.col += csi_arg_count(args[0]);
        at_phantom = false;
        break;

    case 'b': { // REP - ECMA-48 8.3.103
        if(combine_width < 1)
            break;
        const int32_t rw = this_row_width();
        int32_t count = csi_arg_count(args[0]);
        int32_t col = std::min(pos.col + count * combine_width, rw);
        while (pos.col + combine_width <= col) {
            putglyph(make_span(combine_chars.data(), combine_count), combine_width, pos);
            pos.col += combine_width;
        }
        if (pos.col + combine_width >= rw) {
            if (mode.autowrap) {
                at_phantom = true;
                cancel_phantom = false;
            }
        }
        break;
    }

    case 'c': // DA - ECMA-48 8.3.24
        if(csi_arg_or(args[0], 0) == 0)
            vt.push_output_ctrl( C1::CSI, "?1;2c");
        break;

    case LEADER('>', 'c'): // DEC secondary Device Attributes
        vt.push_output_ctrl( C1::CSI, ">{};{};{}c", 0, da2_firmware_version, 0);
        break;

    case 'd': // VPA - ECMA-48 8.3.158
        pos.row = csi_arg_or(args[0], 1) - 1;
        if(mode.origin)
            pos.row += scrollregion_top;
        at_phantom = false;
        break;

    case 'e': // VPR - ECMA-48 8.3.160
        pos.row += csi_arg_count(args[0]);
        at_phantom = false;
        break;

    case 'f': { // HVP - ECMA-48 8.3.63
        int32_t row = csi_arg_or(args[0], 1);
        int32_t col = argcount < 2 || csi_arg_is_missing(args[1]) ? 1 : csi_arg_i32(args[1]);
        // zero-based
        pos.row = row-1;
        pos.col = col-1;
        if(mode.origin) {
            pos.row += scrollregion_top;
            pos.col += scrollregion_left_val();
        }
        at_phantom = false;
        break;
    }

    case 'g': { // TBC - ECMA-48 8.3.154
        int32_t val = csi_arg_or(args[0], 0);

        switch(val) {
        case 0:
            clear_col_tabstop(pos.col);
            break;
        case 3:
        case 5:
            for(int32_t col = 0; col < cols; col++)
                clear_col_tabstop(col);
            break;
        case 1:
        case 2:
        case 4:
            break;
        // TODO: 1, 2 and 4 aren't meaningful yet without line tab stops
        default:
            return false;
        }
        break;
    }

    case 'h': // SM - ECMA-48 8.3.125
        if(!csi_arg_is_missing(args[0]))
            set_mode(csi_arg_i32(args[0]), true);
        break;

    case LEADER('?', 'h'): // DEC private mode set
        for(int32_t i = 0; i < argcount; i++) {
            if(!csi_arg_is_missing(args[i]))
                set_dec_mode(csi_arg_i32(args[i]), true);
        }
        break;

    case 'j': // HPB - ECMA-48 8.3.58
        pos.col -= csi_arg_count(args[0]);
        at_phantom = false;
        break;

    case 'k': // VPB - ECMA-48 8.3.159
        pos.row -= csi_arg_count(args[0]);
        at_phantom = false;
        break;

    case 'l': // RM - ECMA-48 8.3.106
        if(!csi_arg_is_missing(args[0]))
            set_mode(csi_arg_i32(args[0]), false);
        break;

    case LEADER('?', 'l'): // DEC private mode reset
        for(int32_t i = 0; i < argcount; i++) {
            if(!csi_arg_is_missing(args[i]))
                set_dec_mode(csi_arg_i32(args[i]), false);
        }
        break;

    case 'm': // SGR - ECMA-48 8.3.117
        setpen(args);
        break;

    case LEADER('?', 'm'): // DECSGR
        // No actual DEC terminal recognised these, but some printers did. These
        // are alternative ways to request subscript/superscript/off
        for(int32_t argi = 0; argi < argcount; argi++) {
            switch(csi_arg(args[argi])) {
                case 4: { // Superscript on
                    int64_t a = 73;
                    setpen(std::span{&a, 1});
                    break;
                }
                case 5: { // Subscript on
                    int64_t a = 74;
                    setpen(std::span{&a, 1});
                    break;
                }
                case 24: { // Super+subscript off
                    int64_t a = 75;
                    setpen(std::span{&a, 1});
                    break;
                }
            }
        }
        break;

    case 'n': // DSR - ECMA-48 8.3.35
    case LEADER('?', 'n'): { // DECDSR
        int32_t val = csi_arg_or(args[0], 0);

        {
            std::string_view qmark = (leader_byte == '?') ? "?" : "";

            switch(val) {
            case 0: case 1: case 2: case 3: case 4:
                // ignore - these are replies
                break;
            case 5:
                vt.push_output_ctrl(C1::CSI, "{}0n", qmark);
                break;
            case 6: // CPR - cursor position report
                vt.push_output_ctrl(C1::CSI, "{}{};{}R", qmark, pos.row + 1, pos.col + 1);
                break;
            }
        }
        break;
    }

    case INTERMED('!', 'p'): // DECSTR - DEC soft terminal reset
        reset(false);
        break;

    case LEADER('?', INTERMED('$', 'p')):
        request_dec_mode(csi_arg_i32(args[0]));
        break;

    case LEADER('>', 'q'): // XTVERSION - xterm query version string
        request_version_string();
        break;

    case INTERMED(' ', 'q'): { // DECSCUSR - DEC set cursor shape
        struct CursorStyleEntry { bool blink; CursorShape shape; };
        static constexpr std::array<CursorStyleEntry, 7> decscusr_styles = {{
            {true,  CursorShape::Block},     // 0
            {true,  CursorShape::Block},     // 1
            {false, CursorShape::Block},     // 2
            {true,  CursorShape::Underline}, // 3
            {false, CursorShape::Underline}, // 4
            {true,  CursorShape::BarLeft},   // 5
            {false, CursorShape::BarLeft},   // 6
        }};

        int32_t val = csi_arg_or(args[0], 1);
        if(val >= 0 && val < static_cast<int32_t>(decscusr_styles.size())) {
            (void)settermprop_bool(Prop::CursorBlink, decscusr_styles[val].blink);
            (void)settermprop_int (Prop::CursorShape, to_underlying(decscusr_styles[val].shape));
        }

        break;
    }

    case INTERMED('"', 'q'): { // DECSCA - DEC select character protection attribute
        int32_t val = csi_arg_or(args[0], 0);

        switch(val) {
        case 0: case 2:
            protected_cell = false;
            break;
        case 1:
            protected_cell = true;
            break;
        }

        break;
    }

    case 'r': // DECSTBM - DEC custom
        scrollregion_top = csi_arg_or(args[0], 1) - 1;
        scrollregion_bottom = argcount < 2 || csi_arg_is_missing(args[1]) ? -1 : csi_arg_i32(args[1]);
        scrollregion_top = std::clamp(scrollregion_top, 0, rows);
        scrollregion_bottom = std::max(scrollregion_bottom, -1);
        if(scrollregion_top == 0 && scrollregion_bottom == rows)
            scrollregion_bottom = scrollregion_unset;
        else
            scrollregion_bottom = std::min(scrollregion_bottom, rows);

        if(scrollregion_bottom_val() <= scrollregion_top) {
            // Invalid
            scrollregion_top    = 0;
            scrollregion_bottom = scrollregion_unset;
        }

        // Setting the scrolling region restores the cursor to the home position
        pos.row = 0;
        pos.col = 0;
        if(mode.origin) {
            pos.row += scrollregion_top;
            pos.col += scrollregion_left_val();
        }

        break;

    case 's': // DECSLRM - DEC custom
        // Always allow setting these margins, just they won't take effect without DECVSSM
        scrollregion_left = csi_arg_or(args[0], 1) - 1;
        scrollregion_right = argcount < 2 || csi_arg_is_missing(args[1]) ? -1 : csi_arg_i32(args[1]);
        scrollregion_left = std::clamp(scrollregion_left, 0, cols);
        scrollregion_right = std::max(scrollregion_right, -1);
        if(scrollregion_left == 0 && scrollregion_right == cols)
            scrollregion_right = scrollregion_unset;
        else
            scrollregion_right = std::min(scrollregion_right, cols);

        if((scrollregion_right != scrollregion_unset && scrollregion_right <= scrollregion_left) ||
           scrollregion_left >= cols) {
            // Invalid
            scrollregion_left  = 0;
            scrollregion_right = scrollregion_unset;
        }

        // Setting the scrolling region restores the cursor to the home position
        pos.row = 0;
        pos.col = 0;
        if(mode.origin) {
            pos.row += scrollregion_top;
            pos.col += scrollregion_left_val();
        }

        break;

    case INTERMED('\'', '}'): { // DECIC
        int32_t count = csi_arg_count(args[0]);

        if(!is_cursor_in_scrollregion())
            break;

        Rect rect{.start_row = scrollregion_top, .end_row = scrollregion_bottom_val(), .start_col = pos.col, .end_col = scrollregion_right_val()};
        scroll(rect, 0, -count);

        break;
    }

    case INTERMED('\'', '~'): { // DECDC
        int32_t count = csi_arg_count(args[0]);

        if(!is_cursor_in_scrollregion())
            break;

        Rect rect{.start_row = scrollregion_top, .end_row = scrollregion_bottom_val(), .start_col = pos.col, .end_col = scrollregion_right_val()};
        scroll(rect, 0, count);

        break;
    }

    default:
        if(fallbacks)
            if(fallbacks->on_csi(leader, args, intermed, command))
                return true;

        return false;
    }


    if(mode.origin) {
        pos.row = std::clamp(pos.row, scrollregion_top, scrollregion_bottom_val()-1);
        pos.col = std::clamp(pos.col, scrollregion_left_val(), scrollregion_right_val()-1);
    }
    else {
        pos.row = std::clamp(pos.row, 0, rows-1);
        pos.col = std::max(0, std::min(pos.col, this_row_width()-1));
    }

    updatecursor(oldpos, cancel_phantom);

#ifdef DEBUG
    if(pos.row < 0 || pos.row >= rows ||
       pos.col < 0 || pos.col >= cols) {
        std::cerr << std::format("Position out of bounds after CSI {:c}: ({},{})\n",
            command, pos.row, pos.col);
        std::abort();
    }

    if(scrollregion_bottom_val() <= scrollregion_top) {
        std::cerr << std::format("Scroll region height out of bounds after CSI {:c}: {} <= {}\n",
            command, scrollregion_bottom_val(), scrollregion_top);
        std::abort();
    }

    if(scrollregion_right_val() <= scrollregion_left_val()) {
        std::cerr << std::format("Scroll region width out of bounds after CSI {:c}: {} <= {}\n",
            command, scrollregion_right_val(), scrollregion_left_val());
        std::abort();
    }
#endif

    return true;
}

// ---- Base64 helpers ----

namespace {

constexpr uint8_t  base64_invalid           = 0xFF;
constexpr uint32_t base64_partial_mask_18bit = 0x03'FFFF;
constexpr uint32_t base64_partial_mask_24bit = 0xFF'FFFF;
constexpr int32_t  base64_partial_count_shift = 24;

constexpr auto base64_encode_table = [] {
    std::array<char, 64> t{};
    for(int32_t i = 0; i < 26; i++) t[i]      = 'A' + i;
    for(int32_t i = 0; i < 26; i++) t[26 + i]  = 'a' + i;
    for(int32_t i = 0; i < 10; i++) t[52 + i]  = '0' + i;
    t[62] = '+';
    t[63] = '/';
    return t;
}();

constexpr auto base64_decode_table = [] {
    std::array<uint8_t, 256> t{};
    t.fill(base64_invalid);
    for(int32_t i = 0; i < 26; i++) t['A' + i] = i;
    for(int32_t i = 0; i < 26; i++) t['a' + i] = 26 + i;
    for(int32_t i = 0; i < 10; i++) t['0' + i] = 52 + i;
    t['+'] = 62;
    t['/'] = 63;
    return t;
}();

[[nodiscard]] constexpr char base64_one(uint8_t b)
{
    return b < 64 ? base64_encode_table[b] : '\0';
}

[[nodiscard]] constexpr uint8_t unbase64one(char c)
{
    return base64_decode_table[static_cast<uint8_t>(c)];
}

} // anonymous namespace

// ---- OSC selection handler ----

void State::Impl::osc_selection(StringFragment frag)
{
    if(frag.initial) {
        tmp.selection.mask = 0;
        tmp.selection.state = SelectionState::Initial;
    }

    while(tmp.selection.state == SelectionState::Initial && !frag.str.empty()) {
        // Parse selection parameter
        switch(frag.str[0]) {
            case 'c':
                tmp.selection.mask |= to_underlying(SelectionMask::Clipboard);
                break;
            case 'p':
                tmp.selection.mask |= to_underlying(SelectionMask::Primary);
                break;
            case 'q':
                tmp.selection.mask |= to_underlying(SelectionMask::Secondary);
                break;
            case 's':
                tmp.selection.mask |= to_underlying(SelectionMask::Select);
                break;
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
                tmp.selection.mask |= (to_underlying(SelectionMask::Cut0) << (frag.str[0] - '0'));
                break;

            case ';':
                tmp.selection.state = SelectionState::Selected;
                if(!tmp.selection.mask)
                    tmp.selection.mask = to_underlying(SelectionMask::Select) | to_underlying(SelectionMask::Cut0);
                break;
        }

        frag.str.remove_prefix(1);
    }

    if(frag.str.empty()) {
        // Clear selection if we're already finished but didn't do anything
        if(frag.final_ && selection.callbacks) {
            SelectionMask mask = selection_mask();
            StringFragment sf{};
            sf.str = {};
            sf.initial = (tmp.selection.state != SelectionState::Set);
            sf.final_ = true;
            selection.callbacks->on_set(mask, sf);
        }
        return;
    }

    if(tmp.selection.state == SelectionState::Selected) {
        if(frag.str[0] == '?') {
            tmp.selection.state = SelectionState::Query;
        }
        else {
            tmp.selection.state = SelectionState::SetInitial;
            tmp.selection.recvpartial = 0;
        }
    }

    if(tmp.selection.state == SelectionState::Query) {
        if(selection.callbacks) {
            SelectionMask mask = selection_mask();
            selection.callbacks->on_query(mask);
        }
        return;
    }

    if(tmp.selection.state == SelectionState::Invalid)
        return;

    if(selection.callbacks) {
        size_t bufcur = 0;

        uint32_t x = 0; // Current decoding value
        int32_t n = 0;  // Number of sextets consumed

        if(tmp.selection.recvpartial) {
            n = tmp.selection.recvpartial >> base64_partial_count_shift;
            x = tmp.selection.recvpartial & base64_partial_mask_18bit;

            tmp.selection.recvpartial = 0;
        }

        while((selection.buffer.size() - bufcur) >= 3 && !frag.str.empty()) {
            if(frag.str[0] == '=') {
                if(n == 2) {
                    selection.buffer[bufcur++] = (x >> 4) & 0xFF;
                }
                if(n == 3) {
                    selection.buffer[bufcur++] = (x >> 10) & 0xFF;
                    selection.buffer[bufcur++] = (x >>  2) & 0xFF;
                }

                while(!frag.str.empty() && frag.str[0] == '=')
                    frag.str.remove_prefix(1);

                n = 0;
            }
            else {
                uint8_t b = unbase64one(frag.str[0]);
                if(b == base64_invalid) {
                    DEBUG_LOG("base64decode bad input {:02X}\n", static_cast<uint8_t>(frag.str[0]));

                    tmp.selection.state = SelectionState::Invalid;
                    if(selection.callbacks) {
                        SelectionMask mask = selection_mask();
                        StringFragment sf{};
                        sf.str = {};
                        sf.initial = true;
                        sf.final_ = true;
                        selection.callbacks->on_set(mask, sf);
                    }
                    break;
                }

                x = (x << 6) | b;
                n++;
                frag.str.remove_prefix(1);

                if(n == 4) {
                    selection.buffer[bufcur++] = (x >> 16) & 0xFF;
                    selection.buffer[bufcur++] = (x >>  8) & 0xFF;
                    selection.buffer[bufcur++] = (x >>  0) & 0xFF;

                    x = 0;
                    n = 0;
                }
            }

            if(frag.str.empty() || (selection.buffer.size() - bufcur) < 3) {
                if(bufcur) {
                    SelectionMask mask = selection_mask();
                    StringFragment sf{};
                    sf.str = std::string_view{selection.buffer.data(), bufcur};
                    sf.initial = (tmp.selection.state == SelectionState::SetInitial);
                    sf.final_ = (frag.final_ && frag.str.empty());
                    selection.callbacks->on_set(mask, sf);
                    tmp.selection.state = SelectionState::Set;
                }

                bufcur = 0;
            }
        }

        if(n)
            tmp.selection.recvpartial = (n << base64_partial_count_shift) | x;
    }
}

// ---- OSC handler ----

bool State::Impl::on_osc(int32_t command, StringFragment frag)
{
    switch(command) {
        case 0:
            (void)settermprop_string(Prop::IconName, frag);
            (void)settermprop_string(Prop::Title, frag);
            return true;

        case 1:
            (void)settermprop_string(Prop::IconName, frag);
            return true;

        case 2:
            (void)settermprop_string(Prop::Title, frag);
            return true;

        case 52:
            if(selection.callbacks)
                osc_selection(frag);

            return true;

        default:
            if(fallbacks)
                if(fallbacks->on_osc(command, frag))
                    return true;
    }

    return false;
}

// ---- DCS status string request ----

void State::Impl::request_status_string(StringFragment frag)
{
    Terminal::Impl& vt = this->vt;

    auto& buf = this->tmp.decrqss;

    if(frag.initial)
        buf.fill(0);

    size_t i = 0;
    while(i < buf.size()-1 && buf[i])
        i++;
    while(i < buf.size()-1 && !frag.str.empty()) {
        buf[i++] = frag.str[0];
        frag.str.remove_prefix(1);
    }
    buf[i] = 0;

    if(!frag.final_)
        return;

    auto cmd = std::string_view{buf.data(), i};

    if(cmd == "m") {
        // Query SGR
        std::array<int64_t, 20> args{};
        int32_t argc = getpen(args);

        std::string s;
        if(vt.mode.ctrl8bit)
            s += '\x90';
        else {
            s += esc_c;
            s += 'P';
        }
        s += "1$r";

        for(int32_t argi = 0; argi < argc; argi++) {
            s += std::format("{}", csi_arg(args[argi]));
            if(argi < argc - 1)
                s += csi_arg_has_more(args[argi]) ? ':' : ';';
        }

        s += 'm';
        if(vt.mode.ctrl8bit)
            s += '\x9C';
        else {
            s += esc_c;
            s += '\\';
        }

        vt.push_output_bytes(s);
        return;
    }
    else if(cmd == " q") {
        // Query DECSCUSR
        int32_t reply;
        switch(static_cast<CursorShape>(mode.cursor_shape)) {
            case CursorShape::Block:     reply = 2; break;
            case CursorShape::Underline: reply = 4; break;
            case CursorShape::BarLeft:   reply = 6; break;
            default:                     reply = 2; break;
        }
        if(mode.cursor_blink)
            reply--;
        vt.push_output_str( C1::DCS, true,
            "1$r{} q", reply);
        return;
    }
    else if(cmd == "\"q") {
        // Query DECSCA
        vt.push_output_str( C1::DCS, true,
            "1$r{}\"q", protected_cell ? 1 : 2);
        return;
    }
    else if(cmd == "r") {
        // Query DECSTBM
        vt.push_output_str( C1::DCS, true,
            "1$r{};{}r", scrollregion_top+1, scrollregion_bottom_val());
        return;
    }
    else if(cmd == "s") {
        // Query DECSLRM
        vt.push_output_str( C1::DCS, true,
            "1$r{};{}s", scrollregion_left_val()+1, scrollregion_right_val());
        return;
    }

    this->vt.push_output_str( C1::DCS, true, "0$r");
}

// ---- DCS handler ----

bool State::Impl::on_dcs(std::string_view command, StringFragment frag)
{
    if(command == "$q") {
        request_status_string(frag);
        return true;
    }
    else if(fallbacks)
        if(fallbacks->on_dcs(command, frag))
            return true;

    DEBUG_LOG("libvterm: Unhandled DCS {}\n", command);
    return false;
}

// ---- APC/PM/SOS handlers ----

bool State::Impl::on_apc(StringFragment frag)
{
    if(fallbacks)
        if(fallbacks->on_apc(frag))
            return true;

    // No DEBUG_LOG because all APCs are unhandled
    return false;
}

bool State::Impl::on_pm(StringFragment frag)
{
    if(fallbacks)
        if(fallbacks->on_pm(frag))
            return true;

    // No DEBUG_LOG because all PMs are unhandled
    return false;
}

bool State::Impl::on_sos(StringFragment frag)
{
    if(fallbacks)
        if(fallbacks->on_sos(frag))
            return true;

    // No DEBUG_LOG because all SOSs are unhandled
    return false;
}

// ---- Resize handler ----

bool State::Impl::on_resize(int32_t rows, int32_t cols)
{
    Pos oldpos = pos;

    if(cols != this->cols) {
        std::vector<uint8_t> newtabstops((cols + tabstop_bit_mask) / (tabstop_bit_mask + 1));

        size_t common_bytes = std::min(tabstops.size(), newtabstops.size());
        std::copy_n(tabstops.begin(), common_bytes, newtabstops.begin());

        int32_t col;
        for(col = this->cols; col < cols; col++) {
            uint8_t mask = 1 << (col & tabstop_bit_mask);
            if(col % default_tabstop_interval == 0)
                newtabstops[col >> tabstop_byte_shift] |= mask;
            else
                newtabstops[col >> tabstop_byte_shift] &= ~mask;
        }

        tabstops = std::move(newtabstops);
    }

    int32_t old_rows = this->rows;

    this->rows = rows;
    this->cols = cols;

    if(scrollregion_bottom != scrollregion_unset) {
        if(scrollregion_bottom > this->rows)
            scrollregion_bottom = this->rows;
    }
    if(scrollregion_right != scrollregion_unset) {
        if(scrollregion_right > this->cols)
            scrollregion_right = this->cols;
    }

    // Reset scroll region if top/left are now beyond the new dimensions,
    // or if clamping bottom/right made the region degenerate
    if(scrollregion_top >= this->rows ||
       (scrollregion_bottom != scrollregion_unset && scrollregion_bottom <= scrollregion_top)) {
        scrollregion_top = 0;
        scrollregion_bottom = scrollregion_unset;
    }
    if(scrollregion_left >= this->cols ||
       (scrollregion_right != scrollregion_unset && scrollregion_right <= scrollregion_left)) {
        scrollregion_left = 0;
        scrollregion_right = scrollregion_unset;
    }

    StateFields fields{};
    fields.pos = pos;
    fields.lineinfos[0] = &lineinfos[0];
    fields.lineinfos[1] = &lineinfos[1];

    if(callbacks && callbacks->on_resize(rows, cols, fields)) {
        pos = fields.pos;

        lineinfos[0] = *fields.lineinfos[0];
        lineinfos[1] = *fields.lineinfos[1];
    }
    else {
        if(rows != old_rows) {
            for(int32_t bufidx = bufidx_primary; bufidx <= bufidx_altscreen; bufidx++) {
                if(lineinfos[bufidx].empty())
                    continue;

                std::vector<LineInfo> newlineinfo(rows);

                int32_t row;
                for(row = 0; row < old_rows && row < rows; row++) {
                    newlineinfo[row] = lineinfos[bufidx][row];
                }

                lineinfos[bufidx] = std::move(newlineinfo);
            }
        }
    }

    lineinfo_bufidx = mode.alt_screen ? bufidx_altscreen : bufidx_primary;

    if(at_phantom && pos.col < cols-1) {
        at_phantom = false;
        pos.col++;
    }

    pos.row = std::clamp(pos.row, 0, rows - 1);
    pos.col = std::clamp(pos.col, 0, cols - 1);

    updatecursor(oldpos, true);

    return true;
}

// ---- ParserCallbacks subclass ----

namespace {
class StateParserCallbacks : public ParserCallbacks {
public:
    explicit StateParserCallbacks(State::Impl& s) : state_(s) {}

    int32_t on_text(std::span<const char> bytes) override {
        return state_.on_text(bytes);
    }
    bool on_control(uint8_t control) override {
        return state_.on_control(control);
    }
    bool on_escape(std::string_view bytes) override {
        return state_.on_escape(bytes);
    }
    bool on_csi(std::string_view leader, std::span<const int64_t> args, std::string_view intermed, char command) override {
        return state_.on_csi(leader, args, intermed, command);
    }
    bool on_osc(int32_t command, StringFragment frag) override {
        return state_.on_osc(command, frag);
    }
    bool on_dcs(std::string_view command, StringFragment frag) override {
        return state_.on_dcs(command, frag);
    }
    bool on_apc(StringFragment frag) override {
        return state_.on_apc(frag);
    }
    bool on_pm(StringFragment frag) override {
        return state_.on_pm(frag);
    }
    bool on_sos(StringFragment frag) override {
        return state_.on_sos(frag);
    }
    bool on_resize(int32_t rows, int32_t cols) override {
        return state_.on_resize(rows, cols);
    }

private:
    State::Impl& state_;
};
} // anonymous namespace

// ---- State allocation ----

State::Impl::Impl(Terminal::Impl& vt_ref)
    : vt(vt_ref), rows(vt_ref.rows), cols(vt_ref.cols)
{
    newpen();
    combine_chars.resize(initial_combine_size);
    tabstops.resize((cols + tabstop_bit_mask) / (tabstop_bit_mask + 1));
    lineinfos[bufidx_primary].resize(rows);
    lineinfos[bufidx_altscreen].resize(rows);
    lineinfo_bufidx = bufidx_primary;
    encoding_utf8 = create_encoding(EncodingType::UTF8, 'u');
    encoding_utf8->init();
}

// ---- State::Impl destructor ----

State::Impl::~Impl()
{
    vt.parser.callbacks = nullptr;
}

// ---- obtain_state / state_reset ----

State::Impl& Terminal::Impl::obtain_state()
{
    if(state)
        return *state;

    state = std::make_unique<State::Impl>(*this);
    State::Impl& st = *state;

    st.owned_parser_callbacks = std::make_unique<StateParserCallbacks>(st);
    parser.callbacks = st.owned_parser_callbacks.get();

    return st;
}

void State::Impl::reset(bool hard)
{
    scrollregion_top = 0;
    scrollregion_bottom = scrollregion_unset;
    scrollregion_left = 0;
    scrollregion_right = scrollregion_unset;

    mode.keypad          = false;
    mode.cursor          = false;
    mode.autowrap        = true;
    mode.insert          = false;
    mode.newline         = false;
    mode.alt_screen      = false;
    mode.origin          = false;
    mode.leftrightmargin = false;
    mode.bracketpaste    = false;
    mode.report_focus    = false;

    mouse_flags = 0;

    vt.mode.ctrl8bit   = false;

    for(int32_t col = 0; col < cols; col++)
        if(col % default_tabstop_interval == 0)
            set_col_tabstop(col);
        else
            clear_col_tabstop(col);

    for(int32_t row = 0; row < rows; row++)
        set_lineinfo(row, true, DWL::Off, DHL::Off);

    if(callbacks)
        callbacks->on_initpen();

    resetpen();

    EncodingType default_type = vt.mode.utf8 ? EncodingType::UTF8 : EncodingType::Single94;
    char default_desig = vt.mode.utf8 ? 'u' : 'B';

    for(size_t i = 0; i < encoding.size(); i++) {
        encoding[i] = create_encoding(default_type, default_desig);
        encoding[i]->init();
    }

    gl_set = 0;
    gr_set = 1;
    gsingle_set = 0;

    protected_cell = false;

    // Initialise the props
    (void)settermprop_bool(Prop::CursorVisible, true);
    (void)settermprop_bool(Prop::CursorBlink,   true);
    (void)settermprop_int (Prop::CursorShape,   to_underlying(CursorShape::Block));

    if(hard) {
        pos.row = 0;
        pos.col = 0;
        at_phantom = false;

        Rect rect{.start_row = 0, .end_row = rows, .start_col = 0, .end_col = cols};
        erase(rect, false);
    }
}

// ---- set_termprop_internal ----

bool State::Impl::set_termprop_internal(Prop prop, const Value& val)
{
    // Only store the new value of the property if usercode said it was happy.
    // This is especially important for altscreen switching
    if(callbacks)
        if(!callbacks->on_settermprop(prop, val))
            return false;

    switch(prop) {
    case Prop::Title:
    case Prop::IconName:
        // we don't store these, just transparently pass through
        return true;
    case Prop::CursorVisible:
        mode.cursor_visible = val.boolean;
        return true;
    case Prop::CursorBlink:
        mode.cursor_blink = val.boolean;
        return true;
    case Prop::CursorShape:
        mode.cursor_shape = val.number;
        return true;
    case Prop::Reverse:
        mode.screen = val.boolean;
        return true;
    case Prop::AltScreen:
        mode.alt_screen = val.boolean;
        lineinfo_bufidx = mode.alt_screen ? bufidx_altscreen : bufidx_primary;
        if(mode.alt_screen) {
            Rect rect{.start_row = 0, .end_row = rows, .start_col = 0, .end_col = cols};
            erase(rect, false);
        }
        return true;
    case Prop::Mouse:
        mouse_flags = 0;
        if(val.number)
            mouse_flags |= mouse_want_click;
        if(val.number == to_underlying(MouseProp::Drag))
            mouse_flags |= mouse_want_drag;
        if(val.number == to_underlying(MouseProp::Move))
            mouse_flags |= mouse_want_move;
        return true;
    case Prop::FocusReport:
        mode.report_focus = val.boolean;
        return true;

    case Prop::NProps:
        return false;
    }

    return false;
}

// ---- State public API methods ----

void State::set_callbacks(StateCallbacks& cb)
{
    impl_->callbacks = &cb;
    impl_->callbacks->on_initpen();
}

void State::clear_callbacks()
{
    impl_->callbacks = nullptr;
}

void State::set_fallbacks(StateFallbacks& fb)
{
    impl_->fallbacks = &fb;
}

void State::clear_fallbacks()
{
    impl_->fallbacks = nullptr;
}

void State::enable_premove()
{
    impl_->callbacks_has_premove = true;
}

void State::reset(bool hard)
{
    impl_->reset(hard);
}

Pos State::cursor_pos() const
{
    return impl_->pos;
}

const LineInfo& State::get_lineinfo(int32_t row) const
{
    return impl_->get_lineinfo(row);
}

bool State::set_termprop(Prop prop, Value& val)
{
    return impl_->set_termprop_internal(prop, val);
}

void State::focus_in()
{
    if(impl_->mode.report_focus)
        impl_->vt.push_output_ctrl(C1::CSI, "I");
}

void State::focus_out()
{
    if(impl_->mode.report_focus)
        impl_->vt.push_output_ctrl(C1::CSI, "O");
}

void State::set_selection_callbacks(SelectionCallbacks& cb, size_t buflen)
{
    impl_->selection.callbacks = &cb;
    if(buflen)
        impl_->selection.buffer.resize(buflen);
    else
        impl_->selection.buffer.clear();
}

void State::clear_selection_callbacks()
{
    impl_->selection.callbacks = nullptr;
    impl_->selection.buffer.clear();
}

void State::send_selection(SelectionMask mask, StringFragment frag)
{
    Terminal::Impl& vt = impl_->vt;

    if(frag.initial) {
        // TODO: support sending more than one mask bit
        static constexpr std::string_view selection_chars = "cpqs01234567";
        int32_t idx = std::countr_zero(static_cast<uint16_t>(to_underlying(mask)));
        if(idx >= static_cast<int32_t>(selection_chars.size()))
            idx = 3; // Default to SELECT

        vt.push_output_str( C1::OSC, false, "52;{:c};", selection_chars[idx]);

        impl_->tmp.selection.sendpartial = 0;
    }

    if(!frag.str.empty()) {
        size_t bufcur = 0;

        uint32_t x = 0;
        int32_t n = 0;

        if(impl_->tmp.selection.sendpartial) {
            n = impl_->tmp.selection.sendpartial >> base64_partial_count_shift;
            x = impl_->tmp.selection.sendpartial & base64_partial_mask_24bit;

            impl_->tmp.selection.sendpartial = 0;
        }

        auto& buf = impl_->selection.buffer;

        while((buf.size() - bufcur) >= 4 && !frag.str.empty()) {
            x = (x << 8) | static_cast<uint8_t>(frag.str[0]);
            n++;
            frag.str.remove_prefix(1);

            if(n == 3) {
                buf[bufcur++] = base64_one((x >> 18) & 0x3F);
                buf[bufcur++] = base64_one((x >> 12) & 0x3F);
                buf[bufcur++] = base64_one((x >>  6) & 0x3F);
                buf[bufcur++] = base64_one((x >>  0) & 0x3F);

                x = 0;
                n = 0;
            }

            if(frag.str.empty() || (buf.size() - bufcur) < 4) {
                if(bufcur)
                    vt.push_output_bytes(std::span<const char>{buf.data(), bufcur});

                bufcur = 0;
            }
        }

        if(n)
            impl_->tmp.selection.sendpartial = (n << base64_partial_count_shift) | x;
    }

    if(frag.final_) {
        if(impl_->tmp.selection.sendpartial) {
            int32_t n      = impl_->tmp.selection.sendpartial >> base64_partial_count_shift;
            uint32_t x = impl_->tmp.selection.sendpartial & base64_partial_mask_24bit;

            // n is either 1 or 2 now
            x <<= (n == 1) ? 16 : 8;

            auto& buf = impl_->selection.buffer;
            buf[0] = base64_one((x >> 18) & 0x3F);
            buf[1] = base64_one((x >> 12) & 0x3F);
            buf[2] = (n == 1) ? '=' : base64_one((x >>  6) & 0x3F);
            buf[3] = '=';

            vt.push_output_str( C1::None, true, "{}", std::string_view(buf.data(), 4));
        }
        else
            vt.push_output_str( C1::None, true, "");
    }
}

} // namespace vterm
