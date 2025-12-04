#include <cstdint>

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include "cobs.h"

TEST(Cobs, EncodeNullAndEmptyBuffers)
{
    std::uint8_t data[1U];
    std::uint8_t encoded[] = {0x01U, 0x00U};
    std::uint8_t buffer[sizeof(encoded)];

    ASSERT_EQ(SIZE_MAX, Cobs_Encode(NULL, sizeof(data), buffer, sizeof(buffer)));
    ASSERT_EQ(SIZE_MAX, Cobs_Encode(data, sizeof(data), NULL, sizeof(buffer)));
    ASSERT_EQ(SIZE_MAX, Cobs_Encode(data, 0U, buffer, 0U));
    ASSERT_EQ(SIZE_MAX, Cobs_Encode(data, sizeof(data), buffer, 0U));
    ASSERT_EQ(2U, Cobs_Encode(data, 0U, buffer, sizeof(buffer)));
    ASSERT_THAT(buffer, testing::ElementsAreArray(encoded));
}

TEST(Cobs, EncodeZero)
{
    std::uint8_t data[] = {0x00U};
    std::uint8_t encoded[] = {0x01U, 0x01U, 0x00U};
    std::uint8_t buffer[sizeof(encoded)];

    ASSERT_EQ(sizeof(encoded), Cobs_Encode(data, sizeof(data), buffer, sizeof(buffer)));
    ASSERT_THAT(buffer, testing::ElementsAreArray(encoded));
}

TEST(Cobs, EncodeZeros)
{
    std::uint8_t data[] = {0x00U, 0x00U};
    std::uint8_t encoded[] = {0x01U, 0x01U, 0x01U, 0x00U};
    std::uint8_t buffer[sizeof(encoded)];

    ASSERT_EQ(sizeof(encoded), Cobs_Encode(data, sizeof(data), buffer, sizeof(buffer)));
    ASSERT_THAT(buffer, testing::ElementsAreArray(encoded));
}

TEST(Cobs, EncodeZeroStartEnd)
{
    std::uint8_t data[] = {0x00U, 0x11U, 0x00U};
    std::uint8_t encoded[] = {0x01U, 0x02U, 0x11U, 0x01U, 0x00U};
    std::uint8_t buffer[sizeof(encoded)];

    ASSERT_EQ(sizeof(encoded), Cobs_Encode(data, sizeof(data), buffer, sizeof(buffer)));
    ASSERT_THAT(buffer, testing::ElementsAreArray(encoded));
}

TEST(Cobs, EncodeZeroMiddle)
{
    std::uint8_t data[] = {0x11U, 0x22U, 0x00U, 0x33U};
    std::uint8_t encoded[] = {0x03U, 0x11U, 0x22U, 0x02U, 0x33U, 0x00U};
    std::uint8_t buffer[sizeof(encoded)];

    ASSERT_EQ(sizeof(encoded), Cobs_Encode(data, sizeof(data), buffer, sizeof(buffer)));
    ASSERT_THAT(buffer, testing::ElementsAreArray(encoded));
}

TEST(Cobs, EncodeNoZeros)
{
    std::uint8_t data[] = {0x11U, 0x22U, 0x33U, 0x44U};
    std::uint8_t encoded[] = {0x05U, 0x11U, 0x22U, 0x33U, 0x44U, 0x00U};
    std::uint8_t buffer[sizeof(encoded)];

    ASSERT_EQ(sizeof(encoded), Cobs_Encode(data, sizeof(data), buffer, sizeof(buffer)));
    ASSERT_THAT(buffer, testing::ElementsAreArray(encoded));
}

TEST(Cobs, EncodeTrailingZeros)
{
    std::uint8_t data[] = {0x11U, 0x00U, 000U, 0x00U};
    std::uint8_t encoded[] = {0x02U, 0x11U, 0x01U, 0x01U, 0x01U, 0x00U};
    std::uint8_t buffer[sizeof(encoded)];

    ASSERT_EQ(sizeof(encoded), Cobs_Encode(data, sizeof(data), buffer, sizeof(buffer)));
    ASSERT_THAT(buffer, testing::ElementsAreArray(encoded));
}

TEST(Cobs, EncodeNoZerosMaxCode)
{
    std::uint8_t data[254U];
    std::uint8_t encoded[sizeof(data) + 2U] = {0x00U};
    std::uint8_t buffer[sizeof(encoded)];

    for (std::size_t i = 1U; i <= sizeof(data); i++)
    {
        data[i - 1U] = i;
        encoded[i] = i;
    }
    encoded[0U] = 0xFFU;
    encoded[sizeof(encoded) - 1U] = 0x00U;

    ASSERT_EQ(sizeof(encoded), Cobs_Encode(data, sizeof(data), buffer, sizeof(buffer)));
    ASSERT_THAT(buffer, testing::ElementsAreArray(encoded));
}

TEST(Cobs, EncodeZeroStartMaxCode)
{
    std::uint8_t data[255U];
    std::uint8_t encoded[sizeof(data) + 2U] = {0x00U};
    std::uint8_t buffer[sizeof(encoded)];

    for (std::size_t i = 0U; i < sizeof(data); i++)
    {
        data[i] = i;
        encoded[i + 1U] = i;
    }
    encoded[0U] = 0x01U;
    encoded[1U] = 0xFFU;
    encoded[sizeof(encoded) - 1U] = 0x00U;

    ASSERT_EQ(sizeof(encoded), Cobs_Encode(data, sizeof(data), buffer, sizeof(buffer)));
    ASSERT_THAT(buffer, testing::ElementsAreArray(encoded));
}

TEST(Cobs, EncodeNoZeroMaxCodeRollover)
{
    std::uint8_t data[255U];
    std::uint8_t encoded[sizeof(data) + 3U] = {0x00U};
    std::uint8_t buffer[sizeof(encoded)];

    for (std::size_t i = 1U; i <= sizeof(data); i++)
    {
        data[i - 1U] = i;
        encoded[i] = i;
    }
    encoded[0U] = 0xFFU;
    encoded[sizeof(encoded) - 3U] = 0x02U;
    encoded[sizeof(encoded) - 2U] = 0xFFU;
    encoded[sizeof(encoded) - 1U] = 0x00U;

    ASSERT_EQ(sizeof(encoded), Cobs_Encode(data, sizeof(data), buffer, sizeof(buffer)));
    ASSERT_THAT(buffer, testing::ElementsAreArray(encoded));
}

TEST(Cobs, EncodeZeroEndMaxCodeRollover)
{
    std::uint8_t data[255U];
    std::uint8_t encoded[sizeof(data) + 3U] = {0x00U};
    std::uint8_t buffer[sizeof(encoded)];

    for (std::size_t i = 0U; i < sizeof(data); i++)
    {
        data[i] = i + 2U;
        encoded[i + 1U] = i + 2U;
    }
    data[sizeof(data)] = 0x00U;
    encoded[0U] = 0xFFU;
    encoded[sizeof(encoded) - 3U] = 0x01U;
    encoded[sizeof(encoded) - 2U] = 0x01U;
    encoded[sizeof(encoded) - 1U] = 0x00U;

    ASSERT_EQ(sizeof(encoded), Cobs_Encode(data, sizeof(data), buffer, sizeof(buffer)));
    ASSERT_THAT(buffer, testing::ElementsAreArray(encoded));
}

TEST(Cobs, EncodeZeroEndRestart)
{
    std::uint8_t data[255U];
    std::uint8_t encoded[sizeof(data) + 2U] = {0x00U};
    std::uint8_t buffer[sizeof(encoded)];

    for (std::size_t i = 0U; i < sizeof(data); i++)
    {
        data[i] = static_cast<std::uint8_t>(i + 3U);
        encoded[i + 1U] = static_cast<std::uint8_t>(i + 3U);
    }
    encoded[0U] = 0xFEU;
    encoded[sizeof(encoded) - 3U] = 0x02U;
    encoded[sizeof(encoded) - 2U] = 0x01U;
    encoded[sizeof(encoded) - 1U] = 0x00U;

    ASSERT_EQ(sizeof(encoded), Cobs_Encode(data, sizeof(data), buffer, sizeof(buffer)));
    ASSERT_THAT(buffer, testing::ElementsAreArray(encoded));
}

TEST(Cobs, DecodeNullAndEmptyBuffers)
{
    std::uint8_t decoded[1U];
    std::uint8_t buffer[] = {0x01U, 0x00U};
    std::uint8_t data[sizeof(decoded)];

    ASSERT_EQ(SIZE_MAX, Cobs_Decode(NULL, sizeof(data), buffer, sizeof(buffer)));
    ASSERT_EQ(SIZE_MAX, Cobs_Decode(data, sizeof(data), NULL, sizeof(buffer)));
    ASSERT_EQ(SIZE_MAX, Cobs_Decode(data, 0U, buffer, 0U));
    ASSERT_EQ(SIZE_MAX, Cobs_Decode(data, sizeof(data), buffer, 0U));
    ASSERT_EQ(SIZE_MAX, Cobs_Decode(data, sizeof(data), buffer, 1U));
    ASSERT_EQ(0U, Cobs_Decode(data, 0U, buffer, sizeof(buffer)));
    ASSERT_EQ(0U, Cobs_Decode(data, sizeof(data), buffer, sizeof(buffer)));
}

TEST(Cobs, DecodeBadFrame)
{
    std::uint8_t decoded[1U];
    std::uint8_t buffer[] = {0x00U};
    std::uint8_t data[sizeof(decoded)];

    ASSERT_EQ(SIZE_MAX, Cobs_Decode(data, sizeof(data), buffer, sizeof(buffer)));
}

TEST(Cobs, DecodeZero)
{
    std::uint8_t decoded[] = {0x00U};
    std::uint8_t buffer[] = {0x01U, 0x01U, 0x00U};
    std::uint8_t data[sizeof(decoded)];

    ASSERT_EQ(sizeof(decoded), Cobs_Decode(data, sizeof(data), buffer, sizeof(buffer)));
    ASSERT_THAT(data, testing::ElementsAreArray(decoded));
}

TEST(Cobs, DecodeZeros)
{
    std::uint8_t decoded[] = {0x00U, 0x00U};
    std::uint8_t buffer[] = {0x01U, 0x01U, 0x01U, 0x00U};
    std::uint8_t data[sizeof(decoded)];

    ASSERT_EQ(sizeof(decoded), Cobs_Decode(data, sizeof(data), buffer, sizeof(buffer)));
    ASSERT_THAT(data, testing::ElementsAreArray(decoded));
}

TEST(Cobs, DecodeZeroStartEnd)
{
    std::uint8_t decoded[] = {0x00U, 0x11U, 0x00U};
    std::uint8_t buffer[] = {0x01U, 0x02U, 0x11U, 0x01U, 0x00U};
    std::uint8_t data[sizeof(decoded)];

    ASSERT_EQ(sizeof(decoded), Cobs_Decode(data, sizeof(data), buffer, sizeof(buffer)));
    ASSERT_THAT(data, testing::ElementsAreArray(decoded));
}

TEST(Cobs, DecodeZeroMiddle)
{
    std::uint8_t decoded[] = {0x11U, 0x22U, 0x00U, 0x33U};
    std::uint8_t buffer[] = {0x03U, 0x11U, 0x22U, 0x02U, 0x33U, 0x00U};
    std::uint8_t data[sizeof(decoded)];

    ASSERT_EQ(sizeof(decoded), Cobs_Decode(data, sizeof(data), buffer, sizeof(buffer)));
    ASSERT_THAT(data, testing::ElementsAreArray(decoded));
}

TEST(Cobs, DecodeNoZeros)
{
    std::uint8_t decoded[] = {0x11U, 0x22U, 0x33U, 0x44U};
    std::uint8_t buffer[] = {0x05U, 0x11U, 0x22U, 0x33U, 0x44U, 0x00U};
    std::uint8_t data[sizeof(decoded)];

    ASSERT_EQ(sizeof(decoded), Cobs_Decode(data, sizeof(data), buffer, sizeof(buffer)));
    ASSERT_THAT(data, testing::ElementsAreArray(decoded));
}

TEST(Cobs, DecodeTrailingZeros)
{
    std::uint8_t decoded[] = {0x11U, 0x00U, 000U, 0x00U};
    std::uint8_t buffer[] = {0x02U, 0x11U, 0x01U, 0x01U, 0x01U, 0x00U};
    std::uint8_t data[sizeof(decoded)];

    ASSERT_EQ(sizeof(decoded), Cobs_Decode(data, sizeof(data), buffer, sizeof(buffer)));
    ASSERT_THAT(data, testing::ElementsAreArray(decoded));
}

TEST(Cobs, DecodeNoZerosMaxCode)
{
    std::uint8_t decoded[254U];
    std::uint8_t buffer[sizeof(decoded) + 2U] = {0x00U};
    std::uint8_t data[sizeof(decoded)];

    for (std::size_t i = 1U; i <= sizeof(decoded); i++)
    {
        decoded[i - 1U] = i;
        buffer[i] = i;
    }
    buffer[0U] = 0xFFU;
    buffer[sizeof(buffer) - 1U] = 0x00U;

    ASSERT_EQ(sizeof(decoded), Cobs_Decode(data, sizeof(data), buffer, sizeof(buffer)));
    ASSERT_THAT(data, testing::ElementsAreArray(decoded));
}

TEST(Cobs, DecodeZeroStartMaxCode)
{
    std::uint8_t decoded[255U];
    std::uint8_t buffer[sizeof(decoded) + 2U] = {0x00U};
    std::uint8_t data[sizeof(decoded)];

    for (std::size_t i = 0U; i < sizeof(decoded); i++)
    {
        decoded[i] = i;
        buffer[i + 1U] = i;
    }
    buffer[0U] = 0x01U;
    buffer[1U] = 0xFFU;
    buffer[sizeof(buffer) - 1U] = 0x00U;

    ASSERT_EQ(sizeof(decoded), Cobs_Decode(data, sizeof(data), buffer, sizeof(buffer)));
    ASSERT_THAT(data, testing::ElementsAreArray(decoded));
}

TEST(Cobs, DecodeNoZeroMaxCodeRollover)
{
    std::uint8_t decoded[255U];
    std::uint8_t buffer[sizeof(decoded) + 3U] = {0x00U};
    std::uint8_t data[sizeof(decoded)];

    for (std::size_t i = 1U; i <= sizeof(decoded); i++)
    {
        decoded[i - 1U] = i;
        buffer[i] = i;
    }
    buffer[0U] = 0xFFU;
    buffer[sizeof(buffer) - 3U] = 0x02U;
    buffer[sizeof(buffer) - 2U] = 0xFFU;
    buffer[sizeof(buffer) - 1U] = 0x00U;

    ASSERT_EQ(sizeof(decoded), Cobs_Decode(data, sizeof(data), buffer, sizeof(buffer)));
    ASSERT_THAT(data, testing::ElementsAreArray(decoded));
}

TEST(Cobs, DecodeZeroEndMaxCodeRollover)
{
    std::uint8_t decoded[255U];
    std::uint8_t buffer[sizeof(decoded) + 3U] = {0x00U};
    std::uint8_t data[sizeof(decoded)];

    for (std::size_t i = 0U; i < sizeof(decoded); i++)
    {
        decoded[i] = i + 2U;
        buffer[i + 1U] = i + 2U;
    }
    decoded[sizeof(decoded)] = 0x00U;
    buffer[0U] = 0xFFU;
    buffer[sizeof(buffer) - 3U] = 0x01U;
    buffer[sizeof(buffer) - 2U] = 0x01U;
    buffer[sizeof(buffer) - 1U] = 0x00U;

    ASSERT_EQ(sizeof(decoded), Cobs_Decode(data, sizeof(data), buffer, sizeof(buffer)));
    ASSERT_THAT(data, testing::ElementsAreArray(decoded));
}

TEST(Cobs, DecodeZeroEndRestart)
{
    std::uint8_t decoded[255U];
    std::uint8_t buffer[sizeof(decoded) + 2U] = {0x00U};
    std::uint8_t data[sizeof(decoded)];

    for (std::size_t i = 0U; i < sizeof(decoded); i++)
    {
        decoded[i] = static_cast<std::uint8_t>(i + 3U);
        buffer[i + 1U] = static_cast<std::uint8_t>(i + 3U);
    }
    buffer[0U] = 0xFEU;
    buffer[sizeof(buffer) - 3U] = 0x02U;
    buffer[sizeof(buffer) - 2U] = 0x01U;
    buffer[sizeof(buffer) - 1U] = 0x00U;

    ASSERT_EQ(sizeof(decoded), Cobs_Decode(data, sizeof(data), buffer, sizeof(buffer)));
    ASSERT_THAT(data, testing::ElementsAreArray(decoded));
}