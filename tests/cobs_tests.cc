#include <cstdint>

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include "cobs.h"

TEST(Cobs, EncodeNullAndEmptyBuffers)
{
    std::uint8_t data[1U];
    std::uint8_t buffer[sizeof(data)/sizeof(data[0U])];

    ASSERT_EQ(SIZE_MAX, Cobs_Encode(NULL, sizeof(data), buffer, sizeof(buffer)));
    ASSERT_EQ(SIZE_MAX, Cobs_Encode(data, sizeof(data), NULL, sizeof(buffer)));
    ASSERT_EQ(SIZE_MAX, Cobs_Encode(data, 0U, buffer, sizeof(buffer)));
    ASSERT_EQ(SIZE_MAX, Cobs_Encode(data, sizeof(data), buffer, 0U));
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