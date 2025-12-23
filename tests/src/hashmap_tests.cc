#include <cstdint>
#include <span>

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include "err.h"
#include "hashmap.h"

#define SPAN_FROM_VALUE(value, size) std::span<std::uint8_t>(static_cast<std::uint8_t *>(value), size)

TEST(Hashmap, Initialize)
{
    a_Hashmap_t hashmap;
    std::uint8_t data[(sizeof(std::uint32_t) + 10U) * 35U];

    ASSERT_EQ(A_ERR_NULL, a_Hashmap_Initialize(NULL, data, sizeof(data), sizeof(std::uint32_t), 10U));
    ASSERT_EQ(A_ERR_NULL, a_Hashmap_Initialize(&hashmap, NULL, sizeof(data), sizeof(std::uint32_t), 10U));

    ASSERT_EQ(A_ERR_SIZE, a_Hashmap_Initialize(&hashmap, data, 0U, sizeof(std::uint32_t), 10U));
    ASSERT_EQ(A_ERR_SIZE, a_Hashmap_Initialize(&hashmap, data, sizeof(data), 0U, 10U));
    ASSERT_EQ(A_ERR_SIZE, a_Hashmap_Initialize(&hashmap, data, sizeof(data), sizeof(std::uint32_t), 0U));

    ASSERT_EQ(A_ERR_NONE, a_Hashmap_Initialize(&hashmap, data, sizeof(data), sizeof(std::uint32_t), 10U));
    ASSERT_EQ(7U, hashmap.rows);
    ASSERT_EQ(5U, hashmap.columns);

    ASSERT_EQ(A_ERR_NONE, a_Hashmap_Initialize(&hashmap, data, (sizeof(std::uint32_t) + 10U) * 24U, sizeof(std::uint32_t), 10U));
    ASSERT_EQ(6U, hashmap.rows);
    ASSERT_EQ(4U, hashmap.columns);

    ASSERT_EQ(A_ERR_NONE, a_Hashmap_Initialize(&hashmap, data, (sizeof(std::uint32_t) + 10U) * 16U, sizeof(std::uint32_t), 10U));
    ASSERT_EQ(4U, hashmap.rows);
    ASSERT_EQ(4U, hashmap.columns);

    ASSERT_EQ(A_ERR_NONE, a_Hashmap_Initialize(&hashmap, data, (sizeof(std::uint32_t) + 10U) * 11U, sizeof(std::uint32_t), 10U));
    ASSERT_EQ(11U, hashmap.rows);
    ASSERT_EQ(1U, hashmap.columns);
}

TEST(Hashmap, Insert)
{
    a_Hashmap_t hashmap;
    std::uint8_t data[(sizeof(std::uint32_t) + 10U) * 16U];
    std::uint32_t key = 0x12345678U;
    std::uint8_t value[10U] = {0x00U, 0x01U, 0x02U, 0x03U, 0x04U, 0x05U, 0x06U, 0x07U, 0x08U, 0x09U};
    a_Hashmap_Initialize(&hashmap, data, (sizeof(std::uint32_t) + 10U) * 16U, sizeof(std::uint32_t), 10U);

    ASSERT_EQ(A_ERR_NULL, a_Hashmap_Insert(NULL, &key, value));
    ASSERT_EQ(A_ERR_NULL, a_Hashmap_Insert(&hashmap, NULL, value));
    ASSERT_EQ(A_ERR_NULL, a_Hashmap_Insert(&hashmap, &key, NULL));

    ASSERT_EQ(A_ERR_NONE, a_Hashmap_Insert(&hashmap, &key, value));

    /* TODO test handling collisions and if table is full */
}

TEST(Hashmap, Get)
{
    a_Hashmap_t hashmap;
    std::uint8_t data[(sizeof(std::uint32_t) + 10U) * 16U];
    std::uint32_t key = 0x12345678U;
    std::uint8_t value[10U] = {0x00U, 0x01U, 0x02U, 0x03U, 0x04U, 0x05U, 0x06U, 0x07U, 0x08U, 0x09U};
    a_Hashmap_Initialize(&hashmap, data, (sizeof(std::uint32_t) + 10U) * 16U, sizeof(std::uint32_t), 10U);
    a_Hashmap_Insert(&hashmap, &key, value);

    ASSERT_EQ(NULL, a_Hashmap_Get(NULL, &key));
    ASSERT_EQ(NULL, a_Hashmap_Get(&hashmap, NULL));

    ASSERT_THAT(SPAN_FROM_VALUE(a_Hashmap_Get(&hashmap, &key), sizeof(value)), testing::ElementsAreArray(value));

    /* TODO */
}

TEST(Hashmap, Remove)
{
    a_Hashmap_t hashmap;
    std::uint8_t data[(sizeof(std::uint32_t) + 10U) * 16U];
    std::uint32_t key = 0x12345678U;
    std::uint8_t value[10U] = {0x00U, 0x01U, 0x02U, 0x03U, 0x04U, 0x05U, 0x06U, 0x07U, 0x08U, 0x09U};
    a_Hashmap_Initialize(&hashmap, data, (sizeof(std::uint32_t) + 10U) * 16U, sizeof(std::uint32_t), 10U);

    ASSERT_EQ(A_ERR_NULL, a_Hashmap_Remove(NULL, &key));
    ASSERT_EQ(A_ERR_NULL, a_Hashmap_Remove(&hashmap, NULL));

    ASSERT_EQ(A_ERR_NONE, a_Hashmap_Remove(&hashmap, &key));

    a_Hashmap_Insert(&hashmap, &key, value);
    ASSERT_EQ(A_ERR_NONE, a_Hashmap_Remove(&hashmap, &key));
    ASSERT_EQ(NULL, a_Hashmap_Get(&hashmap, &key));

    /* TODO */
}
