#include "internal.h"

#include <limits>

namespace vterm {

namespace {

// Overflow guard for numeric argument accumulation
constexpr int64_t arg_overflow_limit = 100'000'000;

// Intermediate byte range (ECMA-48 5.4)
constexpr uint8_t intermed_start = 0x20;
constexpr uint8_t intermed_end   = 0x2f;

// CSI leader byte range
constexpr uint8_t csi_leader_start = 0x3c;
constexpr uint8_t csi_leader_end   = 0x3f;

// Final byte ranges
constexpr uint8_t final_start     = 0x40;
constexpr uint8_t c1_final_end    = 0x60; // upper bound (exclusive) for 7-bit C1 final bytes
constexpr uint8_t esc_st_final    = 0x5c; // ESC \ = ST (String Terminator) final byte
constexpr uint8_t esc_final_start = 0x30; // ESC final byte range start
constexpr uint8_t final_end       = 0x7e;

[[nodiscard]] constexpr bool is_intermed(uint8_t c) {
    return c >= intermed_start && c <= intermed_end;
}

} // anonymous namespace

void Terminal::Impl::do_control(uint8_t control) {
    if(parser.callbacks)
        if(parser.callbacks->on_control(control))
            return;

    DEBUG_LOG("libvterm: Unhandled control 0x{:02x}\n", control);
}

void Terminal::Impl::do_csi(char command) {
    if(parser.callbacks)
        if(parser.callbacks->on_csi(
              std::string_view{parser.v.csi.leader.data(), parser.v.csi.leaderlen},
              std::span{parser.v.csi.args}.first(static_cast<size_t>(parser.v.csi.argi)),
              std::string_view{parser.intermed.data(), parser.intermedlen},
              command))
            return;

    DEBUG_LOG("libvterm: Unhandled CSI {:c}\n", command);
}

void Terminal::Impl::do_escape(char command) {
    std::array<char, intermed_max + 1> seq{};

    size_t len = parser.intermedlen;
    std::copy_n(parser.intermed.data(), len, seq.data());
    seq[len++] = command;

    if(parser.callbacks)
        if(parser.callbacks->on_escape(std::string_view{seq.data(), len}))
            return;

    DEBUG_LOG("libvterm: Unhandled escape ESC 0x{:02x}\n", command);
}

void Terminal::Impl::string_fragment(std::span<const char> str, bool final_) {
    StringFragment frag{
        .str     = std::string_view{str.data(), str.size()},
        .initial = parser.string_initial,
        .final_  = final_,
    };

    switch(parser.state) {
    case ParserState::OSC:
        if(parser.callbacks)
            parser.callbacks->on_osc(parser.v.osc.command, frag);
        break;

    case ParserState::DCS:
        if(parser.callbacks)
            parser.callbacks->on_dcs(std::string_view{parser.v.dcs.command.data(), parser.v.dcs.commandlen}, frag);
        break;

    case ParserState::APC:
        if(parser.callbacks)
            parser.callbacks->on_apc(frag);
        break;

    case ParserState::PM:
        if(parser.callbacks)
            parser.callbacks->on_pm(frag);
        break;

    case ParserState::SOS:
        if(parser.callbacks)
            parser.callbacks->on_sos(frag);
        break;

    case ParserState::Normal:
    case ParserState::CSILeader:
    case ParserState::CSIArgs:
    case ParserState::CSIIntermed:
    case ParserState::OSCCommand:
    case ParserState::DCSCommand:
        break;
    }

    parser.string_initial = false;
}

bool Terminal::Impl::is_string_state() const {
    return parser.state >= ParserState::OSCCommand;
}

size_t Terminal::Impl::input_write(std::span<const char> data) {
    static constexpr size_t no_string = std::numeric_limits<size_t>::max();
    size_t pos = 0;
    size_t string_start;

    switch(parser.state) {
    case ParserState::Normal:
    case ParserState::CSILeader:
    case ParserState::CSIArgs:
    case ParserState::CSIIntermed:
    case ParserState::OSCCommand:
    case ParserState::DCSCommand:
        string_start = no_string;
        break;
    case ParserState::OSC:
    case ParserState::DCS:
    case ParserState::APC:
    case ParserState::PM:
    case ParserState::SOS:
        string_start = 0;
        break;
    }

    auto enter_state = [&](ParserState st) { parser.state = st; string_start = no_string; };
    auto enter_normal_state = [&]() { enter_state(ParserState::Normal); };

    for( ; pos < data.size(); pos++) {
        uint8_t c = static_cast<uint8_t>(data[pos]);
        bool c1_allowed = !mode.utf8;

        if(c == ctrl_nul || c == ctrl_del) { // NUL, DEL
            if(is_string_state() && string_start != no_string) {
                string_fragment(data.subspan(string_start, pos - string_start), false);
                string_start = pos + 1;
            }
            if(parser.emit_nul)
                do_control(c);
            continue;
        }
        if(c == ctrl_can || c == ctrl_sub) { // CAN, SUB
            parser.in_esc = false;
            enter_normal_state();
            if(parser.emit_nul)
                do_control(c);
            continue;
        }
        else if(c == ctrl_esc) { // ESC
            parser.intermedlen = 0;
            if(!is_string_state())
                parser.state = ParserState::Normal;
            parser.in_esc = true;
            continue;
        }
        else if(c == ctrl_bel &&  // BEL, can stand for ST in OSC or DCS state
                is_string_state()) {
            // fallthrough
        }
        else if(c < c0_end) { // other C0
            if(parser.state == ParserState::SOS)
                continue; // All other C0s permitted in SOS

            if(is_string_state() && string_start != no_string)
                string_fragment(data.subspan(string_start, pos - string_start), false);
            do_control(c);
            if(is_string_state())
                string_start = pos + 1;
            continue;
        }
        // else fallthrough

        size_t string_len = pos - string_start;

        if(parser.in_esc) {
            if(!parser.intermedlen &&
                c >= final_start && c < c1_final_end &&
                ((!is_string_state() || c == esc_st_final))) {
                c += c1_esc_offset;
                c1_allowed = true;
                if(string_len)
                    string_len -= 1;
                parser.in_esc = false;
            }
            else {
                string_start = no_string;
                parser.state = ParserState::Normal;
            }
        }

        switch(parser.state) {
        case ParserState::CSILeader:
            if(c >= csi_leader_start && c <= csi_leader_end) {
                if(parser.v.csi.leaderlen < csi_leader_max - 1)
                    parser.v.csi.leader[parser.v.csi.leaderlen++] = c;
                break;
            }

            parser.v.csi.leader[parser.v.csi.leaderlen] = 0;
            parser.v.csi.argi = 0;
            parser.v.csi.args[0] = csi_arg_missing;
            parser.state = ParserState::CSIArgs;

            [[fallthrough]];
        case ParserState::CSIArgs:
            if(c >= '0' && c <= '9') {
                if(parser.v.csi.args[parser.v.csi.argi] == csi_arg_missing)
                    parser.v.csi.args[parser.v.csi.argi] = 0;
                else if(parser.v.csi.args[parser.v.csi.argi] > arg_overflow_limit)
                    break;
                parser.v.csi.args[parser.v.csi.argi] *= 10;
                parser.v.csi.args[parser.v.csi.argi] += c - '0';
                break;
            }
            if(c == ':') {
                parser.v.csi.args[parser.v.csi.argi] |= csi_arg_flag_more;
                c = ';';
            }
            if(c == ';') {
                if(parser.v.csi.argi < csi_args_max - 1) {
                    parser.v.csi.argi++;
                    parser.v.csi.args[parser.v.csi.argi] = csi_arg_missing;
                }
                break;
            }

            parser.v.csi.argi++;
            parser.intermedlen = 0;
            parser.state = ParserState::CSIIntermed;
            [[fallthrough]];
        case ParserState::CSIIntermed:
            if(is_intermed(c)) {
                if(parser.intermedlen < intermed_max - 1)
                    parser.intermed[parser.intermedlen++] = c;
                break;
            }
            else if(c == ctrl_esc) {
                // ESC in CSI cancels
            }
            else if(c >= final_start && c <= final_end) {
                parser.intermed[parser.intermedlen] = 0;
                do_csi(c);
            }

            enter_normal_state();
            break;

        case ParserState::OSCCommand:
            if(c >= '0' && c <= '9') {
                if(parser.v.osc.command == -1)
                    parser.v.osc.command = 0;
                else if(parser.v.osc.command > arg_overflow_limit)
                    break;
                else
                    parser.v.osc.command *= 10;
                parser.v.osc.command += c - '0';
                break;
            }
            if(c == ';') {
                parser.state = ParserState::OSC;
                string_start = pos + 1;
                break;
            }

            string_start = pos;
            string_len   = 0;
            parser.state = ParserState::OSC;
            // Inline string_state: check for string termination
            if(c == ctrl_bel || (c1_allowed && c == to_underlying(C1::ST))) {
                string_fragment(data.subspan(string_start, string_len), true);
                enter_normal_state();
            }
            break;

        case ParserState::DCSCommand:
            if(parser.v.dcs.commandlen < csi_leader_max)
                parser.v.dcs.command[parser.v.dcs.commandlen++] = c;

            if(c >= final_start && c <= final_end) {
                string_start = pos + 1;
                parser.state = ParserState::DCS;
            }
            break;

        case ParserState::OSC:
        case ParserState::DCS:
        case ParserState::APC:
        case ParserState::PM:
        case ParserState::SOS:
            if(c == ctrl_bel || (c1_allowed && c == to_underlying(C1::ST))) {
                string_fragment(data.subspan(string_start, string_len), true);
                enter_normal_state();
            }
            break;

        case ParserState::Normal:
            if(parser.in_esc) {
                if(is_intermed(c)) {
                    if(parser.intermedlen < intermed_max - 1)
                        parser.intermed[parser.intermedlen++] = c;
                }
                else if(c >= esc_final_start && c <= final_end) {
                    do_escape(c);
                    parser.in_esc = false;
                    enter_normal_state();
                }
                else {
                    DEBUG_LOG("TODO: Unhandled byte {:02x} in Escape\n", c);
                }
                break;
            }
            if(c1_allowed && c >= c1_start && c < c1_end) {
                switch(static_cast<C1>(c)) {
                case C1::DCS:
                    parser.string_initial = true;
                    parser.v.dcs.commandlen = 0;
                    enter_state(ParserState::DCSCommand);
                    break;
                case C1::SOS:
                    parser.string_initial = true;
                    enter_state(ParserState::SOS);
                    string_start = pos + 1;
                    string_len = 0;
                    break;
                case C1::CSI:
                    parser.v.csi.leaderlen = 0;
                    enter_state(ParserState::CSILeader);
                    break;
                case C1::OSC:
                    parser.v.osc.command = -1;
                    parser.string_initial = true;
                    string_start = pos + 1;
                    enter_state(ParserState::OSCCommand);
                    break;
                case C1::PM:
                    parser.string_initial = true;
                    enter_state(ParserState::PM);
                    string_start = pos + 1;
                    string_len = 0;
                    break;
                case C1::APC:
                    parser.string_initial = true;
                    enter_state(ParserState::APC);
                    string_start = pos + 1;
                    string_len = 0;
                    break;
                default:
                    do_control(c);
                    break;
                }
            }
            else {
                size_t eaten = 0;
                if(parser.callbacks)
                    eaten = parser.callbacks->on_text(data.subspan(pos));

                if(eaten == 0) {
                    DEBUG_LOG("libvterm: Text callback did not consume any input\n");
                    eaten = 1;
                }

                pos += (eaten - 1); // we'll ++ it again in a moment
            }
            break;
        }
    }

    if(string_start != no_string) {
        size_t string_len = pos - string_start;
        if(parser.in_esc && string_len > 0)
            string_len -= 1;
        string_fragment(data.subspan(string_start, string_len), false);
    }

    return data.size();
}

} // namespace vterm
