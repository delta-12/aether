#include <cstdint>

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include "leb128.h"

template <typename T>
constexpr std::size_t kMaxBufferSize = ((sizeof(T) * 8U) / 7U) + 1U;

TEST(Leb128, EncodeNullandEmptyBuffers)
{
    std::uint8_t buffer[1U];

    ASSERT_EQ(SIZE_MAX, Leb128_Encode8(0U, NULL, sizeof(buffer)));
    ASSERT_EQ(SIZE_MAX, Leb128_Encode16(0U, NULL, sizeof(buffer)));
    ASSERT_EQ(SIZE_MAX, Leb128_Encode32(0U, NULL, sizeof(buffer)));
    ASSERT_EQ(SIZE_MAX, Leb128_Encode64(0U, NULL, sizeof(buffer)));
    ASSERT_EQ(SIZE_MAX, Leb128_Encode8(0U, buffer, 0U));
    ASSERT_EQ(SIZE_MAX, Leb128_Encode16(0U, buffer, 0U));
    ASSERT_EQ(SIZE_MAX, Leb128_Encode32(0U, buffer, 0U));
    ASSERT_EQ(SIZE_MAX, Leb128_Encode64(0U, buffer, 0U));
    ASSERT_EQ(SIZE_MAX, Leb128_Encode8(0U, NULL, 0U));
    ASSERT_EQ(SIZE_MAX, Leb128_Encode16(0U, NULL, 0U));
    ASSERT_EQ(SIZE_MAX, Leb128_Encode32(0U, NULL, 0U));
    ASSERT_EQ(SIZE_MAX, Leb128_Encode64(0U, NULL, 0U));
}

TEST(Leb128, EncodeInsufficientSize)
{
    std::uint8_t buffer[kMaxBufferSize<std::uint64_t>];

    ASSERT_EQ(SIZE_MAX, Leb128_Encode8(UINT8_MAX, buffer, kMaxBufferSize<std::uint8_t> - 1U));
    ASSERT_EQ(SIZE_MAX, Leb128_Encode16(UINT16_MAX, buffer, kMaxBufferSize<std::uint16_t> - 1U));
    ASSERT_EQ(SIZE_MAX, Leb128_Encode32(UINT32_MAX, buffer, kMaxBufferSize<std::uint32_t> - 1U));
    ASSERT_EQ(SIZE_MAX, Leb128_Encode64(UINT64_MAX, buffer, kMaxBufferSize<std::uint64_t> - 1U));
}

TEST(Leb128, Encode8)
{
    std::uint8_t encoded_zero[] = {0x00U};
    std::uint8_t encoded_one[] = {0x01U};
    std::uint8_t encoded_max[] = {0xFFU, 0x01U};
    std::uint8_t encoded_negative_max[] = {0x80U, 0x01U};
    std::uint8_t buffer_min[1U];
    std::uint8_t buffer_max[kMaxBufferSize<std::uint8_t>];

    ASSERT_EQ(sizeof(encoded_zero), Leb128_Encode8(0U, buffer_min, sizeof(buffer_min)));
    ASSERT_THAT(buffer_min, testing::ElementsAreArray(encoded_zero));

    ASSERT_EQ(sizeof(encoded_one), Leb128_Encode8(1U, buffer_min, sizeof(buffer_min)));
    ASSERT_THAT(buffer_min, testing::ElementsAreArray(encoded_one));

    ASSERT_EQ(sizeof(encoded_max), Leb128_Encode8(-1, buffer_max, sizeof(buffer_max)));
    ASSERT_THAT(buffer_max, testing::ElementsAreArray(encoded_max));

    ASSERT_EQ(sizeof(encoded_max), Leb128_Encode8(UINT8_MAX, buffer_max, sizeof(buffer_max)));
    ASSERT_THAT(buffer_max, testing::ElementsAreArray(encoded_max));

    ASSERT_EQ(sizeof(encoded_negative_max), Leb128_Encode8(INT8_MIN, buffer_max, sizeof(buffer_max)));
    ASSERT_THAT(buffer_max, testing::ElementsAreArray(encoded_negative_max));
}

TEST(Leb128, Encode16)
{
    std::uint8_t encoded_zero[] = {0x00U};
    std::uint8_t encoded_one[] = {0x01U};
    std::uint8_t encoded_max[] = {0xFFU, 0xFFU, 0x03U};
    std::uint8_t encoded_negative_max[] = {0x80U, 0x80U, 0x02U};
    std::uint8_t buffer_min[1U];
    std::uint8_t buffer_max[kMaxBufferSize<std::uint16_t>];

    ASSERT_EQ(sizeof(encoded_zero), Leb128_Encode16(0U, buffer_min, sizeof(buffer_min)));
    ASSERT_THAT(buffer_min, testing::ElementsAreArray(encoded_zero));

    ASSERT_EQ(sizeof(encoded_one), Leb128_Encode16(1U, buffer_min, sizeof(buffer_min)));
    ASSERT_THAT(buffer_min, testing::ElementsAreArray(encoded_one));

    ASSERT_EQ(sizeof(encoded_max), Leb128_Encode16(-1, buffer_max, sizeof(buffer_max)));
    ASSERT_THAT(buffer_max, testing::ElementsAreArray(encoded_max));

    ASSERT_EQ(sizeof(encoded_max), Leb128_Encode16(UINT16_MAX, buffer_max, sizeof(buffer_max)));
    ASSERT_THAT(buffer_max, testing::ElementsAreArray(encoded_max));

    ASSERT_EQ(sizeof(encoded_negative_max), Leb128_Encode16(INT16_MIN, buffer_max, sizeof(buffer_max)));
    ASSERT_THAT(buffer_max, testing::ElementsAreArray(encoded_negative_max));
}

TEST(Leb128, Encode32)
{
    std::uint8_t encoded_zero[] = {0x00U};
    std::uint8_t encoded_one[] = {0x01U};
    std::uint8_t encoded_max[] = {0xFFU, 0xFFU, 0xFFU, 0xFFU, 0x0FU};
    std::uint8_t encoded_negative_max[] = {0x80U, 0x80U, 0x80U, 0x80U, 0x08U};
    std::uint8_t buffer_min[1U];
    std::uint8_t buffer_max[kMaxBufferSize<std::uint32_t>];

    ASSERT_EQ(sizeof(encoded_zero), Leb128_Encode32(0U, buffer_min, sizeof(buffer_min)));
    ASSERT_THAT(buffer_min, testing::ElementsAreArray(encoded_zero));

    ASSERT_EQ(sizeof(encoded_one), Leb128_Encode32(1U, buffer_min, sizeof(buffer_min)));
    ASSERT_THAT(buffer_min, testing::ElementsAreArray(encoded_one));

    ASSERT_EQ(sizeof(encoded_max), Leb128_Encode32(-1, buffer_max, sizeof(buffer_max)));
    ASSERT_THAT(buffer_max, testing::ElementsAreArray(encoded_max));

    ASSERT_EQ(sizeof(encoded_max), Leb128_Encode32(UINT32_MAX, buffer_max, sizeof(buffer_max)));
    ASSERT_THAT(buffer_max, testing::ElementsAreArray(encoded_max));

    ASSERT_EQ(sizeof(encoded_negative_max), Leb128_Encode32(INT32_MIN, buffer_max, sizeof(buffer_max)));
    ASSERT_THAT(buffer_max, testing::ElementsAreArray(encoded_negative_max));
}

TEST(Leb128, Encode64)
{
    std::uint8_t encoded_zero[] = {0x00U};
    std::uint8_t encoded_one[] = {0x01U};
    std::uint8_t encoded_max[] = {0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0x01U};
    std::uint8_t encoded_negative_max[] = {0x80U, 0x80U, 0x80U, 0x80U, 0x80U, 0x80U, 0x80U, 0x80U, 0x80U, 0x01U};
    std::uint8_t buffer_min[1U];
    std::uint8_t buffer_max[kMaxBufferSize<std::uint64_t>];

    ASSERT_EQ(sizeof(encoded_zero), Leb128_Encode64(0U, buffer_min, sizeof(buffer_min)));
    ASSERT_THAT(buffer_min, testing::ElementsAreArray(encoded_zero));

    ASSERT_EQ(sizeof(encoded_one), Leb128_Encode64(1U, buffer_min, sizeof(buffer_min)));
    ASSERT_THAT(buffer_min, testing::ElementsAreArray(encoded_one));

    ASSERT_EQ(sizeof(encoded_max), Leb128_Encode64(-1, buffer_max, sizeof(buffer_max)));
    ASSERT_THAT(buffer_max, testing::ElementsAreArray(encoded_max));

    ASSERT_EQ(sizeof(encoded_max), Leb128_Encode64(UINT64_MAX, buffer_max, sizeof(buffer_max)));
    ASSERT_THAT(buffer_max, testing::ElementsAreArray(encoded_max));

    ASSERT_EQ(sizeof(encoded_negative_max), Leb128_Encode64(INT64_MIN, buffer_max, sizeof(buffer_max)));
    ASSERT_THAT(buffer_max, testing::ElementsAreArray(encoded_negative_max));
}

TEST(Leb128, DecodeNullandEmptyBuffers)
{
    std::uint8_t buffer[1U];
    std::uint8_t u8;
    std::uint16_t u16;
    std::uint32_t u32;
    std::uint64_t u64;

    ASSERT_EQ(SIZE_MAX, Leb128_Decode8(&u8, NULL, sizeof(buffer)));
    ASSERT_EQ(SIZE_MAX, Leb128_Decode16(&u16, NULL, sizeof(buffer)));
    ASSERT_EQ(SIZE_MAX, Leb128_Decode32(&u32, NULL, sizeof(buffer)));
    ASSERT_EQ(SIZE_MAX, Leb128_Decode64(&u64, NULL, sizeof(buffer)));
    ASSERT_EQ(SIZE_MAX, Leb128_Decode8(&u8, buffer, 0U));
    ASSERT_EQ(SIZE_MAX, Leb128_Decode16(&u16, buffer, 0U));
    ASSERT_EQ(SIZE_MAX, Leb128_Decode32(&u32, buffer, 0U));
    ASSERT_EQ(SIZE_MAX, Leb128_Decode64(&u64, buffer, 0U));
    ASSERT_EQ(SIZE_MAX, Leb128_Decode8(&u8, NULL, 0U));
    ASSERT_EQ(SIZE_MAX, Leb128_Decode16(&u16, NULL, 0U));
    ASSERT_EQ(SIZE_MAX, Leb128_Decode32(&u32, NULL, 0U));
    ASSERT_EQ(SIZE_MAX, Leb128_Decode64(&u64, NULL, 0U));
    ASSERT_EQ(SIZE_MAX, Leb128_Decode8(NULL, NULL, sizeof(buffer)));
    ASSERT_EQ(SIZE_MAX, Leb128_Decode16(NULL, NULL, sizeof(buffer)));
    ASSERT_EQ(SIZE_MAX, Leb128_Decode32(NULL, NULL, sizeof(buffer)));
    ASSERT_EQ(SIZE_MAX, Leb128_Decode64(NULL, NULL, sizeof(buffer)));
    ASSERT_EQ(SIZE_MAX, Leb128_Decode8(NULL, buffer, 0U));
    ASSERT_EQ(SIZE_MAX, Leb128_Decode16(NULL, buffer, 0U));
    ASSERT_EQ(SIZE_MAX, Leb128_Decode32(NULL, buffer, 0U));
    ASSERT_EQ(SIZE_MAX, Leb128_Decode64(NULL, buffer, 0U));
    ASSERT_EQ(SIZE_MAX, Leb128_Decode8(NULL, NULL, 0U));
    ASSERT_EQ(SIZE_MAX, Leb128_Decode16(NULL, NULL, 0U));
    ASSERT_EQ(SIZE_MAX, Leb128_Decode32(NULL, NULL, 0U));
    ASSERT_EQ(SIZE_MAX, Leb128_Decode64(NULL, NULL, 0U));
}

TEST(Leb128, DecodeBadEncoding)
{
    std::uint8_t buffer[kMaxBufferSize<std::uint64_t>] = {0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU};
    std::uint8_t u8;
    std::uint16_t u16;
    std::uint32_t u32;
    std::uint64_t u64;

    ASSERT_EQ(SIZE_MAX, Leb128_Decode8(&u8, buffer, kMaxBufferSize<std::uint8_t> - 1U));
    ASSERT_EQ(SIZE_MAX, Leb128_Decode16(&u16, buffer, kMaxBufferSize<std::uint16_t> - 1U));
    ASSERT_EQ(SIZE_MAX, Leb128_Decode32(&u32, buffer, kMaxBufferSize<std::uint32_t> - 1U));
    ASSERT_EQ(SIZE_MAX, Leb128_Decode64(&u64, buffer, kMaxBufferSize<std::uint64_t> - 1U));
}

TEST(Leb128, Decode8)
{
    std::uint8_t encoded_zero[] = {0x00U};
    std::uint8_t encoded_one[] = {0x01U};
    std::uint8_t encoded_max[] = {0xFFU, 0x01U};
    std::uint8_t encoded_negative_max[] = {0x80U, 0x01U};
    std::uint8_t value;

    ASSERT_EQ(sizeof(encoded_zero), Leb128_Decode8(&value, encoded_zero, sizeof(encoded_zero)));
    ASSERT_EQ(0U, value);

    ASSERT_EQ(sizeof(encoded_one), Leb128_Decode8(&value, encoded_one, sizeof(encoded_one)));
    ASSERT_EQ(1U, value);
    value = 0U;

    ASSERT_EQ(sizeof(encoded_max), Leb128_Decode8(&value, encoded_max, sizeof(encoded_max)));
    ASSERT_EQ(-1, static_cast<std::int8_t>(value));
    value = 0U;

    ASSERT_EQ(sizeof(encoded_max), Leb128_Decode8(&value, encoded_max, sizeof(encoded_max)));
    ASSERT_EQ(UINT8_MAX, value);
    value = 0U;

    ASSERT_EQ(sizeof(encoded_negative_max), Leb128_Decode8(&value, encoded_negative_max, sizeof(encoded_negative_max)));
    ASSERT_EQ(INT8_MIN, static_cast<std::int8_t>(value));
}

TEST(Leb128, Decode16)
{
    std::uint8_t encoded_zero[] = {0x00U};
    std::uint8_t encoded_one[] = {0x01U};
    std::uint8_t encoded_max[] = {0xFFU, 0xFFU, 0x03U};
    std::uint8_t encoded_negative_max[] = {0x80U, 0x80U, 0x02U};
    std::uint16_t value;

    ASSERT_EQ(sizeof(encoded_zero), Leb128_Decode16(&value, encoded_zero, sizeof(encoded_zero)));
    ASSERT_EQ(0U, value);

    ASSERT_EQ(sizeof(encoded_one), Leb128_Decode16(&value, encoded_one, sizeof(encoded_one)));
    ASSERT_EQ(1U, value);
    value = 0U;

    ASSERT_EQ(sizeof(encoded_max), Leb128_Decode16(&value, encoded_max, sizeof(encoded_max)));
    ASSERT_EQ(-1, static_cast<std::int16_t>(value));
    value = 0U;

    ASSERT_EQ(sizeof(encoded_max), Leb128_Decode16(&value, encoded_max, sizeof(encoded_max)));
    ASSERT_EQ(UINT16_MAX, value);
    value = 0U;

    ASSERT_EQ(sizeof(encoded_negative_max), Leb128_Decode16(&value, encoded_negative_max, sizeof(encoded_negative_max)));
    ASSERT_EQ(INT16_MIN, static_cast<std::int16_t>(value));
}

TEST(Leb128, Decode32)
{
    std::uint8_t encoded_zero[] = {0x00U};
    std::uint8_t encoded_one[] = {0x01U};
    std::uint8_t encoded_max[] = {0xFFU, 0xFFU, 0xFFU, 0xFFU, 0x0FU};
    std::uint8_t encoded_negative_max[] = {0x80U, 0x80U, 0x80U, 0x80U, 0x08U};
    std::uint32_t value;

    ASSERT_EQ(sizeof(encoded_zero), Leb128_Decode32(&value, encoded_zero, sizeof(encoded_zero)));
    ASSERT_EQ(0U, value);

    ASSERT_EQ(sizeof(encoded_one), Leb128_Decode32(&value, encoded_one, sizeof(encoded_one)));
    ASSERT_EQ(1U, value);
    value = 0U;

    ASSERT_EQ(sizeof(encoded_max), Leb128_Decode32(&value, encoded_max, sizeof(encoded_max)));
    ASSERT_EQ(-1, static_cast<std::int32_t>(value));
    value = 0U;

    ASSERT_EQ(sizeof(encoded_max), Leb128_Decode32(&value, encoded_max, sizeof(encoded_max)));
    ASSERT_EQ(UINT32_MAX, value);
    value = 0U;

    ASSERT_EQ(sizeof(encoded_negative_max), Leb128_Decode32(&value, encoded_negative_max, sizeof(encoded_negative_max)));
    ASSERT_EQ(INT32_MIN, static_cast<std::int32_t>(value));
}

TEST(Leb128, Decode64)
{
    std::uint8_t encoded_zero[] = {0x00U};
    std::uint8_t encoded_one[] = {0x01U};
    std::uint8_t encoded_max[] = {0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0x01U};
    std::uint8_t encoded_negative_max[] = {0x80U, 0x80U, 0x80U, 0x80U, 0x80U, 0x80U, 0x80U, 0x80U, 0x80U, 0x01U};
    std::uint64_t value;

    ASSERT_EQ(sizeof(encoded_zero), Leb128_Decode64(&value, encoded_zero, sizeof(encoded_zero)));
    ASSERT_EQ(0U, value);

    ASSERT_EQ(sizeof(encoded_one), Leb128_Decode64(&value, encoded_one, sizeof(encoded_one)));
    ASSERT_EQ(1U, value);
    value = 0U;

    ASSERT_EQ(sizeof(encoded_max), Leb128_Decode64(&value, encoded_max, sizeof(encoded_max)));
    ASSERT_EQ(-1, static_cast<std::int64_t>(value));
    value = 0U;

    ASSERT_EQ(sizeof(encoded_max), Leb128_Decode64(&value, encoded_max, sizeof(encoded_max)));
    ASSERT_EQ(UINT64_MAX, value);
    value = 0U;

    ASSERT_EQ(sizeof(encoded_negative_max), Leb128_Decode64(&value, encoded_negative_max, sizeof(encoded_negative_max)));
    ASSERT_EQ(INT64_MIN, static_cast<std::int64_t>(value));
}
