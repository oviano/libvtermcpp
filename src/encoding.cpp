#include "internal.h"
#include "utf8.h"

namespace vterm {

namespace {

constexpr int32_t unicode_invalid = 0xFFFD;

// Unicode noncharacters that must be rejected by the decoder
constexpr int32_t unicode_nonchar_fffe = 0xFFFE;
constexpr int32_t unicode_nonchar_ffff = 0xFFFF;

// UTF-8 decoder byte range boundaries
constexpr uint8_t decode_c0_end            = 0x20;
constexpr uint8_t decode_ascii_end          = 0x7f;
constexpr uint8_t decode_continuation_start = 0x80;
constexpr uint8_t decode_continuation_end   = 0xc0;
constexpr uint8_t decode_2byte_start        = 0xc0;
constexpr uint8_t decode_3byte_start        = 0xe0;
constexpr uint8_t decode_4byte_start        = 0xf0;
constexpr uint8_t decode_5byte_start        = 0xf8;
constexpr uint8_t decode_6byte_start        = 0xfc;
constexpr uint8_t decode_invalid_start      = 0xfe;

// Unicode surrogate pair range
constexpr int32_t surrogate_start = 0xD800;
constexpr int32_t surrogate_end   = 0xDFFF;

// --- UTF-8 encoding ---

struct UTF8Encoding : EncodingInstance {
    int32_t bytes_remaining = 0;
    int32_t bytes_total = 0;
    int32_t this_cp = 0;

    void init() override {
        bytes_remaining = 0;
        bytes_total     = 0;
    }

    DecodeResult decode(std::span<uint32_t> output, std::span<const char> input) override
    {
        size_t ipos = 0;
        int32_t opos = 0;
        int32_t cplen = static_cast<int32_t>(output.size());

        for(; ipos < input.size() && opos < cplen; ipos++) {
            uint8_t c = input[ipos];

            if(c < decode_c0_end) // C0
                return {opos, ipos};

            else if(c >= decode_c0_end && c < decode_ascii_end) {
                if(bytes_remaining != 0) {
                    output[opos++] = unicode_invalid;
                    bytes_remaining = 0;
                    if(opos >= cplen)
                        return {opos, ipos};
                }
                output[opos++] = c;
                bytes_remaining = 0;
            }

            else if(c == decode_ascii_end) // DEL
                return {opos, ipos};

            else if(c >= decode_continuation_start && c < decode_continuation_end) {
                if(bytes_remaining == 0) {
                    output[opos++] = unicode_invalid;
                    continue;
                }

                this_cp <<= 6;
                this_cp |= c & utf8_continuation_mask;
                bytes_remaining--;

                if(bytes_remaining == 0) {
                    switch(bytes_total) {
                    case 2: if(this_cp < utf8_max_1byte)  this_cp = unicode_invalid; break;
                    case 3: if(this_cp < utf8_max_2byte)  this_cp = unicode_invalid; break;
                    case 4: if(this_cp < utf8_max_3byte)  this_cp = unicode_invalid; break;
                    case 5: if(this_cp < utf8_max_4byte)  this_cp = unicode_invalid; break;
                    case 6: if(this_cp < utf8_max_5byte)  this_cp = unicode_invalid; break;
                    }
                    if((this_cp >= surrogate_start && this_cp <= surrogate_end) ||
                       this_cp == unicode_nonchar_fffe || this_cp == unicode_nonchar_ffff)
                        this_cp = unicode_invalid;
                    output[opos++] = this_cp;
                }
            }

            else if(c >= decode_2byte_start && c < decode_3byte_start) {
                if(bytes_remaining != 0) output[opos++] = unicode_invalid;
                this_cp = c & utf8_lead_mask[2];
                bytes_total = 2;
                bytes_remaining = 1;
            }

            else if(c >= decode_3byte_start && c < decode_4byte_start) {
                if(bytes_remaining != 0) output[opos++] = unicode_invalid;
                this_cp = c & utf8_lead_mask[3];
                bytes_total = 3;
                bytes_remaining = 2;
            }

            else if(c >= decode_4byte_start && c < decode_5byte_start) {
                if(bytes_remaining != 0) output[opos++] = unicode_invalid;
                this_cp = c & utf8_lead_mask[4];
                bytes_total = 4;
                bytes_remaining = 3;
            }

            else if(c >= decode_5byte_start && c < decode_6byte_start) {
                if(bytes_remaining != 0) output[opos++] = unicode_invalid;
                this_cp = c & utf8_lead_mask[5];
                bytes_total = 5;
                bytes_remaining = 4;
            }

            else if(c >= decode_6byte_start && c < decode_invalid_start) {
                if(bytes_remaining != 0) output[opos++] = unicode_invalid;
                this_cp = c & utf8_lead_mask[6];
                bytes_total = 6;
                bytes_remaining = 5;
            }

            else {
                output[opos++] = unicode_invalid;
            }
        }
        return {opos, ipos};
    }
};

// --- US-ASCII encoding ---

struct USASCIIEncoding : EncodingInstance {
    DecodeResult decode(std::span<uint32_t> output, std::span<const char> input) override
    {
        if(input.empty())
            return {0, 0};

        size_t ipos = 0;
        int32_t opos = 0;
        int32_t cplen = static_cast<int32_t>(output.size());
        uint8_t is_gr = static_cast<uint8_t>(input[ipos] & high_bit);

        for(; ipos < input.size() && opos < cplen; ipos++) {
            uint8_t c = input[ipos] ^ is_gr;

            if(c < decode_c0_end || c == decode_ascii_end || c >= decode_continuation_start)
                return {opos, ipos};

            output[opos++] = c;
        }
        return {opos, ipos};
    }
};

// --- Table-based encoding ---

struct TableEncoding : EncodingInstance {
    std::span<const uint32_t> chars;

    explicit TableEncoding(std::span<const uint32_t> table) : chars(table) {}

    DecodeResult decode(std::span<uint32_t> output, std::span<const char> input) override
    {
        if(input.empty())
            return {0, 0};

        size_t ipos = 0;
        int32_t opos = 0;
        int32_t cplen = static_cast<int32_t>(output.size());
        uint8_t is_gr = static_cast<uint8_t>(input[ipos] & high_bit);

        for(; ipos < input.size() && opos < cplen; ipos++) {
            uint8_t c = input[ipos] ^ is_gr;

            if(c < decode_c0_end || c == decode_ascii_end || c >= decode_continuation_start)
                return {opos, ipos};

            if(chars[c] != 0)
                output[opos++] = chars[c];
            else
                output[opos++] = c;
        }
        return {opos, ipos};
    }
};

// --- Static encoding tables ---

constexpr auto dec_drawing_chars = [] {
    std::array<uint32_t, 128> t{};
    t[0x60] = 0x25C6; t[0x61] = 0x2592; t[0x62] = 0x2409; t[0x63] = 0x240C;
    t[0x64] = 0x240D; t[0x65] = 0x240A; t[0x66] = 0x00B0; t[0x67] = 0x00B1;
    t[0x68] = 0x2424; t[0x69] = 0x240B; t[0x6a] = 0x2518; t[0x6b] = 0x2510;
    t[0x6c] = 0x250C; t[0x6d] = 0x2514; t[0x6e] = 0x253C; t[0x6f] = 0x23BA;
    t[0x70] = 0x23BB; t[0x71] = 0x2500; t[0x72] = 0x23BC; t[0x73] = 0x23BD;
    t[0x74] = 0x251C; t[0x75] = 0x2524; t[0x76] = 0x2534; t[0x77] = 0x252C;
    t[0x78] = 0x2502; t[0x79] = 0x2A7D; t[0x7a] = 0x2A7E; t[0x7b] = 0x03C0;
    t[0x7c] = 0x2260; t[0x7d] = 0x00A3; t[0x7e] = 0x00B7;
    return t;
}();

constexpr auto uk_chars = [] {
    std::array<uint32_t, 128> t{};
    t[0x23] = 0x00A3;
    return t;
}();

} // anonymous namespace

// --- Factory ---

[[nodiscard]] std::unique_ptr<EncodingInstance> create_encoding(EncodingType type, char designation) {
    if(type == EncodingType::UTF8 && designation == 'u')
        return std::make_unique<UTF8Encoding>();

    if(type == EncodingType::Single94) {
        switch(designation) {
        case '0': return std::make_unique<TableEncoding>(dec_drawing_chars);
        case 'A': return std::make_unique<TableEncoding>(uk_chars);
        case 'B': return std::make_unique<USASCIIEncoding>();
        }
    }

    return nullptr;
}

} // namespace vterm
