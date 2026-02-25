#include "internal.h"
#include "utf8.h"

namespace vterm {

namespace {

enum class KeycodeType {
    None,
    Literal,
    Tab,
    Enter,
    SS3,
    CSI,
    CSICursor,
    CSINum,
    Keypad,
};

struct KeyCode {
    KeycodeType type = KeycodeType::None;
    char literal = 0;
    int32_t csinum = 0;
};

constexpr auto keycodes = std::to_array<KeyCode>({
    { KeycodeType::None,      0,      0 }, // NONE

    { KeycodeType::Enter,     '\r',   0 }, // ENTER
    { KeycodeType::Tab,       '\t',   0 }, // TAB
    { KeycodeType::Literal,   '\x7f', 0 }, // BACKSPACE == ASCII DEL
    { KeycodeType::Literal,   '\x1b', 0 }, // ESCAPE

    { KeycodeType::CSICursor, 'A',    0 }, // UP
    { KeycodeType::CSICursor, 'B',    0 }, // DOWN
    { KeycodeType::CSICursor, 'D',    0 }, // LEFT
    { KeycodeType::CSICursor, 'C',    0 }, // RIGHT

    { KeycodeType::CSINum,    '~',    2 }, // INS
    { KeycodeType::CSINum,    '~',    3 }, // DEL
    { KeycodeType::CSICursor, 'H',    0 }, // HOME
    { KeycodeType::CSICursor, 'F',    0 }, // END
    { KeycodeType::CSINum,    '~',    5 }, // PAGEUP
    { KeycodeType::CSINum,    '~',    6 }, // PAGEDOWN
});

constexpr auto keycodes_fn = std::to_array<KeyCode>({
    { KeycodeType::None,   0,   0  }, // F0
    { KeycodeType::SS3,    'P', 0  }, // F1
    { KeycodeType::SS3,    'Q', 0  }, // F2
    { KeycodeType::SS3,    'R', 0  }, // F3
    { KeycodeType::SS3,    'S', 0  }, // F4
    { KeycodeType::CSINum, '~', 15 }, // F5
    { KeycodeType::CSINum, '~', 17 }, // F6
    { KeycodeType::CSINum, '~', 18 }, // F7
    { KeycodeType::CSINum, '~', 19 }, // F8
    { KeycodeType::CSINum, '~', 20 }, // F9
    { KeycodeType::CSINum, '~', 21 }, // F10
    { KeycodeType::CSINum, '~', 23 }, // F11
    { KeycodeType::CSINum, '~', 24 }, // F12
});

constexpr auto keycodes_kp = std::to_array<KeyCode>({
    { KeycodeType::Keypad, '0',  'p' }, // KP_0
    { KeycodeType::Keypad, '1',  'q' }, // KP_1
    { KeycodeType::Keypad, '2',  'r' }, // KP_2
    { KeycodeType::Keypad, '3',  's' }, // KP_3
    { KeycodeType::Keypad, '4',  't' }, // KP_4
    { KeycodeType::Keypad, '5',  'u' }, // KP_5
    { KeycodeType::Keypad, '6',  'v' }, // KP_6
    { KeycodeType::Keypad, '7',  'w' }, // KP_7
    { KeycodeType::Keypad, '8',  'x' }, // KP_8
    { KeycodeType::Keypad, '9',  'y' }, // KP_9
    { KeycodeType::Keypad, '*',  'j' }, // KP_MULT
    { KeycodeType::Keypad, '+',  'k' }, // KP_PLUS
    { KeycodeType::Keypad, ',',  'l' }, // KP_COMMA
    { KeycodeType::Keypad, '-',  'm' }, // KP_MINUS
    { KeycodeType::Keypad, '.',  'n' }, // KP_PERIOD
    { KeycodeType::Keypad, '/',  'o' }, // KP_DIVIDE
    { KeycodeType::Keypad, '\n', 'M' }, // KP_ENTER
    { KeycodeType::Keypad, '=',  'X' }, // KP_EQUAL
});

static_assert(keycodes.size() == static_cast<size_t>(Key::PageDown) + 1,
    "keycodes must cover Key::None through Key::PageDown");
static_assert(keycodes_fn.size() == 13, "keycodes_fn must cover F0 through F12");
static_assert(keycodes_kp.size() == static_cast<size_t>(Key::Max) - static_cast<size_t>(Key::KP0),
    "keycodes_kp must cover KP0 through KPEqual");

constexpr uint8_t ascii_ctrl_mask = 0x1f;

} // anonymous namespace

void Terminal::Impl::keyboard_unichar(uint32_t c, Modifier mod) {
    if(c != ' ')
        mod = mod & ~Modifier::Shift;

    if(mod == Modifier::None) {
        std::array<char, utf8_max_seqlen> str;
        int32_t seqlen = fill_utf8(c, str);
        push_output_bytes(std::span{str}.first(static_cast<size_t>(seqlen)));
        return;
    }

    bool needs_CSIu = false;
    switch(c) {
    case 'i': case 'j': case 'm': case '[':
        needs_CSIu = true;
        break;
    case '\\': case ']': case '^': case '_':
        needs_CSIu = false;
        break;
    case ' ':
        needs_CSIu = ((mod & Modifier::Shift) != Modifier::None);
        break;
    default:
        needs_CSIu = (c < 'a' || c > 'z');
    }

    if(needs_CSIu && (mod & ~Modifier::Alt) != Modifier::None) {
        push_output_ctrl(C1::CSI, "{};{}u", c, to_underlying(mod) + 1);
        return;
    }

    if((mod & Modifier::Ctrl) != Modifier::None)
        c &= ascii_ctrl_mask; // maps 'a'-'z' to 0x01-0x1a

    if((mod & Modifier::Alt) != Modifier::None)
        push_output_bytes(std::span<const char>{&esc_c, 1});

    {
        std::array<char, utf8_max_seqlen> str;
        int32_t seqlen = fill_utf8(c, str);
        push_output_bytes(std::span{str}.first(static_cast<size_t>(seqlen)));
    }
}

void Terminal::Impl::emit_key_literal(char literal, int32_t imod) {
    if(imod != 0 && (imod & (to_underlying(Modifier::Shift) | to_underlying(Modifier::Ctrl))))
        push_output_ctrl(C1::CSI, "{};{}u", static_cast<int32_t>(literal), imod + 1);
    else if(imod != 0 && (imod & to_underlying(Modifier::Alt)))
        push_output("{:c}{:c}", esc_c, literal);
    else
        push_output("{:c}", literal);
}

void Terminal::Impl::emit_key_ss3(char literal, int32_t imod) {
    if(imod == 0)
        push_output_ctrl(C1::SS3, "{:c}", literal);
    else
        emit_key_csi(literal, imod);
}

void Terminal::Impl::emit_key_csi(char literal, int32_t imod) {
    if(imod == 0)
        push_output_ctrl(C1::CSI, "{:c}", literal);
    else
        push_output_ctrl(C1::CSI, "1;{}{:c}", imod + 1, literal);
}

void Terminal::Impl::keyboard_key(Key key, Modifier mod) {
    if(key == Key::None)
        return;

    KeyCode k{};
    int32_t ikey = to_underlying(key);

    if(ikey < to_underlying(Key::Function0)) {
        if(static_cast<size_t>(ikey) >= keycodes.size())
            return;
        k = keycodes[ikey];
    }
    else if(ikey >= to_underlying(Key::Function0) && ikey <= to_underlying(Key::FunctionMax)) {
        int32_t fi = ikey - to_underlying(Key::Function0);
        if(static_cast<size_t>(fi) >= keycodes_fn.size())
            return;
        k = keycodes_fn[fi];
    }
    else if(ikey >= to_underlying(Key::KP0)) {
        int32_t ki = ikey - to_underlying(Key::KP0);
        if(static_cast<size_t>(ki) >= keycodes_kp.size())
            return;
        k = keycodes_kp[ki];
    }

    int32_t imod = to_underlying(mod);

    switch(k.type) {
    case KeycodeType::None:
        break;

    case KeycodeType::Tab:
        if(mod == Modifier::Shift)
            push_output_ctrl(C1::CSI, "Z");
        else if((mod & Modifier::Shift) != Modifier::None)
            push_output_ctrl(C1::CSI, "1;{}Z", imod + 1);
        else
            emit_key_literal(k.literal, imod);
        break;

    case KeycodeType::Enter:
        if(state && state->mode.newline)
            push_output_bytes(std::span<const char>{"\r\n", 2});
        else
            emit_key_literal(k.literal, imod);
        break;

    case KeycodeType::Literal:
        emit_key_literal(k.literal, imod);
        break;

    case KeycodeType::SS3:
        emit_key_ss3(k.literal, imod);
        break;

    case KeycodeType::CSI:
        emit_key_csi(k.literal, imod);
        break;

    case KeycodeType::CSINum:
        if(imod == 0)
            push_output_ctrl(C1::CSI, "{}{:c}", k.csinum, k.literal);
        else
            push_output_ctrl(C1::CSI, "{};{}{:c}", k.csinum, imod + 1, k.literal);
        break;

    case KeycodeType::CSICursor:
        if(state && state->mode.cursor)
            emit_key_ss3(k.literal, imod);
        else
            emit_key_csi(k.literal, imod);
        break;

    case KeycodeType::Keypad:
        if(state && state->mode.keypad)
            emit_key_ss3(static_cast<char>(k.csinum), imod);
        else
            emit_key_literal(k.literal, imod);
        break;
    }
}

void Terminal::Impl::keyboard_start_paste() {
    if(state && state->mode.bracketpaste)
        push_output_ctrl(C1::CSI, "200~");
}

void Terminal::Impl::keyboard_end_paste() {
    if(state && state->mode.bracketpaste)
        push_output_ctrl(C1::CSI, "201~");
}

} // namespace vterm
