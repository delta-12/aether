#include <cstdint>

#include <gtest/gtest.h>

#include "err.h"
#include "hashmap.h"

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
    
    ASSERT_EQ(A_ERR_NONE, a_Hashmap_Initialize(&hashmap, data, (sizeof(std::uint32_t) + 10U) * 11U, sizeof(std::uint32_t), 10U));
    ASSERT_EQ(11U, hashmap.rows);
    ASSERT_EQ(1U, hashmap.columns);
}
