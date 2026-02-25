// test_03_encoding_utf8.cpp — UTF-8 encoding layer tests
// Ported from upstream libvterm t/03encoding_utf8.test

#include "harness.h"

// Helper: create a fresh UTF-8 encoding instance.
static std::unique_ptr<EncodingInstance> make_utf8()
{
    auto ei = create_encoding(EncodingType::UTF8, 'u');
    ei->init();
    return ei;
}

// Helper: decode bytes through encoding and store codepoints in output array.
// Returns the number of codepoints decoded.
static int32_t encoding_decode(EncodingInstance *ei,
                           std::string_view bytes,
                           std::span<uint32_t> cp)
{
    auto result = ei->decode(cp, std::span{bytes.data(), bytes.size()});
    return result.codepoints_produced;
}

// Low
TEST(encoding_utf8_low)
{
    auto ei = make_utf8();

    std::array<uint32_t, 16> cp{};
    int32_t n = encoding_decode(ei.get(), {"123", 3}, cp);
    ASSERT_EQ(n, 3);
    ASSERT_EQ(cp[0], 0x31);
    ASSERT_EQ(cp[1], 0x32);
    ASSERT_EQ(cp[2], 0x33);
}

// 2 byte
TEST(encoding_utf8_2byte)
{
    auto ei = make_utf8();

    std::array<uint32_t, 16> cp{};
    int32_t n = encoding_decode(ei.get(), {"\xC2\x80\xDF\xBF", 4}, cp);
    ASSERT_EQ(n, 2);
    ASSERT_EQ(cp[0], 0x0080);
    ASSERT_EQ(cp[1], 0x07FF);
}

// 3 byte
TEST(encoding_utf8_3byte)
{
    auto ei = make_utf8();

    std::array<uint32_t, 16> cp{};
    int32_t n = encoding_decode(ei.get(), {"\xE0\xA0\x80\xEF\xBF\xBD", 6}, cp);
    ASSERT_EQ(n, 2);
    ASSERT_EQ(cp[0], 0x0800);
    ASSERT_EQ(cp[1], 0xFFFD);
}

// 4 byte
TEST(encoding_utf8_4byte)
{
    auto ei = make_utf8();

    std::array<uint32_t, 16> cp{};
    int32_t n = encoding_decode(ei.get(), {"\xF0\x90\x80\x80\xF7\xBF\xBF\xBF", 8}, cp);
    ASSERT_EQ(n, 2);
    ASSERT_EQ(cp[0], 0x10000);
    ASSERT_EQ(cp[1], 0x1fffff);
}

// Early termination
TEST(encoding_utf8_early_termination)
{
    std::array<uint32_t, 16> cp{};
    int32_t n;

    // 2-byte sequence cut short
    auto ei = make_utf8();
    n = encoding_decode(ei.get(), {"\xC2!", 2}, cp);
    ASSERT_EQ(n, 2);
    ASSERT_EQ(cp[0], 0xfffd);
    ASSERT_EQ(cp[1], 0x21);

    // 3-byte sequences cut short
    ei = make_utf8();
    n = encoding_decode(ei.get(), {"\xE0!\xE0\xA0!", 5}, cp);
    ASSERT_EQ(n, 4);
    ASSERT_EQ(cp[0], 0xfffd);
    ASSERT_EQ(cp[1], 0x21);
    ASSERT_EQ(cp[2], 0xfffd);
    ASSERT_EQ(cp[3], 0x21);

    // 4-byte sequences cut short at various points
    ei = make_utf8();
    n = encoding_decode(ei.get(), {"\xF0!\xF0\x90!\xF0\x90\x80!", 9}, cp);
    ASSERT_EQ(n, 6);
    ASSERT_EQ(cp[0], 0xfffd);
    ASSERT_EQ(cp[1], 0x21);
    ASSERT_EQ(cp[2], 0xfffd);
    ASSERT_EQ(cp[3], 0x21);
    ASSERT_EQ(cp[4], 0xfffd);
    ASSERT_EQ(cp[5], 0x21);
}

// Early restart
TEST(encoding_utf8_early_restart)
{
    std::array<uint32_t, 16> cp{};
    int32_t n;

    // 2-byte restart
    auto ei = make_utf8();
    n = encoding_decode(ei.get(), {"\xC2\xC2\x90", 3}, cp);
    ASSERT_EQ(n, 2);
    ASSERT_EQ(cp[0], 0xfffd);
    ASSERT_EQ(cp[1], 0x0090);

    // 3-byte restart
    ei = make_utf8();
    n = encoding_decode(ei.get(), {"\xE0\xC2\x90\xE0\xA0\xC2\x90", 7}, cp);
    ASSERT_EQ(n, 4);
    ASSERT_EQ(cp[0], 0xfffd);
    ASSERT_EQ(cp[1], 0x0090);
    ASSERT_EQ(cp[2], 0xfffd);
    ASSERT_EQ(cp[3], 0x0090);

    // 4-byte restart
    ei = make_utf8();
    n = encoding_decode(ei.get(),
        {"\xF0\xC2\x90\xF0\x90\xC2\x90\xF0\x90\x80\xC2\x90", 12},
        cp);
    ASSERT_EQ(n, 6);
    ASSERT_EQ(cp[0], 0xfffd);
    ASSERT_EQ(cp[1], 0x0090);
    ASSERT_EQ(cp[2], 0xfffd);
    ASSERT_EQ(cp[3], 0x0090);
    ASSERT_EQ(cp[4], 0xfffd);
    ASSERT_EQ(cp[5], 0x0090);
}

// Overlong
TEST(encoding_utf8_overlong)
{
    std::array<uint32_t, 16> cp{};
    int32_t n;

    // 2-byte overlong
    auto ei = make_utf8();
    n = encoding_decode(ei.get(), {"\xC0\x80\xC1\xBF", 4}, cp);
    ASSERT_EQ(n, 2);
    ASSERT_EQ(cp[0], 0xfffd);
    ASSERT_EQ(cp[1], 0xfffd);

    // 3-byte overlong
    ei = make_utf8();
    n = encoding_decode(ei.get(), {"\xE0\x80\x80\xE0\x9F\xBF", 6}, cp);
    ASSERT_EQ(n, 2);
    ASSERT_EQ(cp[0], 0xfffd);
    ASSERT_EQ(cp[1], 0xfffd);

    // 4-byte overlong
    ei = make_utf8();
    n = encoding_decode(ei.get(), {"\xF0\x80\x80\x80\xF0\x8F\xBF\xBF", 8}, cp);
    ASSERT_EQ(n, 2);
    ASSERT_EQ(cp[0], 0xfffd);
    ASSERT_EQ(cp[1], 0xfffd);
}

// UTF-16 Surrogates
TEST(encoding_utf8_utf16_surrogates)
{
    auto ei = make_utf8();

    std::array<uint32_t, 16> cp{};
    int32_t n = encoding_decode(ei.get(), {"\xED\xA0\x80\xED\xBF\xBF", 6}, cp);
    ASSERT_EQ(n, 2);
    ASSERT_EQ(cp[0], 0xfffd);
    ASSERT_EQ(cp[1], 0xfffd);
}

// Split write
TEST(encoding_utf8_split_write)
{
    std::array<uint32_t, 16> cp{};
    int32_t n;

    // 2-byte split: first byte, then second byte
    auto ei = make_utf8();
    n = encoding_decode(ei.get(), {"\xC2", 1}, cp);
    ASSERT_EQ(n, 0);
    n = encoding_decode(ei.get(), {"\xA0", 1}, cp);
    ASSERT_EQ(n, 1);
    ASSERT_EQ(cp[0], 0x000A0);

    // 3-byte split: first byte, then two remaining
    ei = make_utf8();
    n = encoding_decode(ei.get(), {"\xE0", 1}, cp);
    ASSERT_EQ(n, 0);
    n = encoding_decode(ei.get(), {"\xA0\x80", 2}, cp);
    ASSERT_EQ(n, 1);
    ASSERT_EQ(cp[0], 0x00800);

    // 3-byte split: first two bytes, then last byte
    ei = make_utf8();
    n = encoding_decode(ei.get(), {"\xE0\xA0", 2}, cp);
    ASSERT_EQ(n, 0);
    n = encoding_decode(ei.get(), {"\x80", 1}, cp);
    ASSERT_EQ(n, 1);
    ASSERT_EQ(cp[0], 0x00800);

    // 4-byte split: first byte, then three remaining
    ei = make_utf8();
    n = encoding_decode(ei.get(), {"\xF0", 1}, cp);
    ASSERT_EQ(n, 0);
    n = encoding_decode(ei.get(), {"\x90\x80\x80", 3}, cp);
    ASSERT_EQ(n, 1);
    ASSERT_EQ(cp[0], 0x10000);

    // 4-byte split: first two bytes, then two remaining
    ei = make_utf8();
    n = encoding_decode(ei.get(), {"\xF0\x90", 2}, cp);
    ASSERT_EQ(n, 0);
    n = encoding_decode(ei.get(), {"\x80\x80", 2}, cp);
    ASSERT_EQ(n, 1);
    ASSERT_EQ(cp[0], 0x10000);

    // 4-byte split: first three bytes, then last byte
    ei = make_utf8();
    n = encoding_decode(ei.get(), {"\xF0\x90\x80", 3}, cp);
    ASSERT_EQ(n, 0);
    n = encoding_decode(ei.get(), {"\x80", 1}, cp);
    ASSERT_EQ(n, 1);
    ASSERT_EQ(cp[0], 0x10000);
}
