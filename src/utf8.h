#ifndef VTERM_UTF8_H
#define VTERM_UTF8_H

#include <array>
#include <cassert>
#include <cstdint>
#include <cstddef>
#include <span>

namespace vterm {

// UTF-8 sequence length boundaries (max codepoint for each byte count)
inline constexpr int32_t utf8_max_1byte = 0x80;
inline constexpr int32_t utf8_max_2byte = 0x800;
inline constexpr int32_t utf8_max_3byte = 0x1'0000;
inline constexpr int32_t utf8_max_4byte = 0x20'0000;
inline constexpr int32_t utf8_max_5byte = 0x400'0000;

// Maximum bytes in a single UTF-8 encoded codepoint
inline constexpr int32_t utf8_max_seqlen = 6;

// UTF-8 encoding bit patterns
inline constexpr uint8_t utf8_continuation_prefix = 0x80;
inline constexpr uint8_t utf8_continuation_mask   = 0x3f;

// Lead byte prefix and payload mask, indexed by sequence length (index 0-1 unused)
inline constexpr std::array<uint8_t, 7> utf8_lead_prefix = {0x00, 0x00, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc};
inline constexpr std::array<uint8_t, 7> utf8_lead_mask   = {0x00, 0x7f, 0x1f, 0x0f, 0x07, 0x03, 0x01};

[[nodiscard]] constexpr int32_t utf8_seqlen(int32_t codepoint) {
    if(codepoint < utf8_max_1byte) return 1;
    if(codepoint < utf8_max_2byte) return 2;
    if(codepoint < utf8_max_3byte) return 3;
    if(codepoint < utf8_max_4byte) return 4;
    if(codepoint < utf8_max_5byte) return 5;
    return 6;
}

// Does NOT NUL-terminate the buffer
[[nodiscard]] constexpr int32_t fill_utf8(int32_t codepoint, std::span<char> str) {
    int32_t nbytes = utf8_seqlen(codepoint);
    assert(str.size() >= static_cast<size_t>(nbytes));

    // Fill in continuation bytes in reverse order
    for(int32_t b = nbytes; b > 1; b--) {
        str[b-1] = static_cast<char>(utf8_continuation_prefix | (codepoint & utf8_continuation_mask));
        codepoint >>= 6;
    }

    // Lead byte
    str[0] = static_cast<char>(utf8_lead_prefix[nbytes] | (codepoint & utf8_lead_mask[nbytes]));

    return nbytes;
}

} // namespace vterm

#endif // VTERM_UTF8_H
