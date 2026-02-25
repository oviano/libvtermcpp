#include "internal.h"
#include "utf8.h"

namespace vterm {

namespace {
constexpr int32_t x10_coord_offset      = 0x21;
constexpr int32_t x10_coord_max          = 0xff;
constexpr int32_t x10_button_offset      = 0x20;
constexpr int32_t x10_high_button_offset = 0x40;

constexpr int32_t mouse_button1_mask = 0x01;
constexpr int32_t mouse_button2_mask = 0x02;
constexpr int32_t mouse_button3_mask = 0x04;

constexpr int32_t x10_modifier_shift  = 2;
constexpr int32_t x10_release_code    = 3;
constexpr int32_t primary_button_count = 4;
constexpr int32_t total_button_count   = 8;
} // anonymous namespace

void State::Impl::output_mouse(int32_t code, bool pressed, int32_t modifiers, int32_t col, int32_t row) {
    modifiers <<= x10_modifier_shift;

    switch(mouse_protocol) {
    case MouseProtocol::X10:
        if(col + x10_coord_offset > x10_coord_max)
            col = x10_coord_max - x10_coord_offset;
        if(row + x10_coord_offset > x10_coord_max)
            row = x10_coord_max - x10_coord_offset;

        if(!pressed)
            code = x10_release_code;

        vt.push_output_ctrl(C1::CSI, "M{:c}{:c}{:c}",
            static_cast<char>((code | modifiers) + x10_button_offset),
            static_cast<char>(col + x10_coord_offset),
            static_cast<char>(row + x10_coord_offset));
        break;

    case MouseProtocol::UTF8: {
        std::array<char, 3 * utf8_max_seqlen> utf8;
        size_t len = 0;

        if(!pressed)
            code = x10_release_code;

        len += fill_utf8((code | modifiers) + x10_button_offset, std::span{utf8}.subspan(len));
        len += fill_utf8(col + x10_coord_offset, std::span{utf8}.subspan(len));
        len += fill_utf8(row + x10_coord_offset, std::span{utf8}.subspan(len));

        vt.push_output_ctrl(C1::CSI, "M");
        vt.push_output_bytes(std::span<const char>{utf8.data(), len});
        break;
    }

    case MouseProtocol::SGR:
        vt.push_output_ctrl(C1::CSI, "<{};{};{}{:c}",
            code | modifiers, col + 1, row + 1, pressed ? 'M' : 'm');
        break;

    case MouseProtocol::RXVT:
        if(!pressed)
            code = x10_release_code;

        vt.push_output_ctrl(C1::CSI, "{};{};{}M",
            code | modifiers, col + 1, row + 1);
        break;
    }
}

void Terminal::Impl::mouse_move(int32_t row, int32_t col, Modifier mod) {
    if(!state) return;
    auto& st = *state;

    if(col == st.mouse_col && row == st.mouse_row)
        return;

    st.mouse_col = col;
    st.mouse_row = row;

    if(((st.mouse_flags & mouse_want_drag) && st.mouse_buttons) ||
       (st.mouse_flags & mouse_want_move)) {
        int32_t button = st.mouse_buttons & mouse_button1_mask ? 1 :
                         st.mouse_buttons & mouse_button2_mask ? 2 :
                         st.mouse_buttons & mouse_button3_mask ? 3 : 4;
        st.output_mouse(button - 1 + x10_button_offset, true, to_underlying(mod), col, row);
    }
}

void Terminal::Impl::mouse_button(int32_t button, bool pressed, Modifier mod) {
    if(!state) return;
    auto& st = *state;

    int32_t old_buttons = st.mouse_buttons;

    if(button > 0 && button < primary_button_count) {
        if(pressed)
            st.mouse_buttons |= (1 << (button - 1));
        else
            st.mouse_buttons &= ~(1 << (button - 1));
    }

    if(st.mouse_buttons == old_buttons && button < primary_button_count)
        return;

    if(st.mouse_flags == 0)
        return;

    if(button < primary_button_count) {
        st.output_mouse(button - 1, pressed, to_underlying(mod), st.mouse_col, st.mouse_row);
    }
    else if(button < total_button_count) {
        st.output_mouse(button - 4 + x10_high_button_offset, pressed, to_underlying(mod), st.mouse_col, st.mouse_row);
    }
}

} // namespace vterm
