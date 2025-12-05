#include <cstdint>

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include "buffer.h"
#include "types.h"

TEST(Buffer, Initialize)
{
    a_Buffer_t buffer;
    std::uint8_t data[1U];

    ASSERT_EQ(A_ERROR_NULL, a_Buffer_Initialize(NULL, data, sizeof(data)));
    ASSERT_EQ(A_ERROR_NULL, a_Buffer_Initialize(&buffer, NULL, sizeof(data)));
    ASSERT_EQ(A_ERROR_NULL, a_Buffer_Initialize(NULL, NULL, sizeof(data)));
    ASSERT_EQ(A_ERROR_NONE, a_Buffer_Initialize(&buffer, data, 0U));
    ASSERT_EQ(A_ERROR_NONE, a_Buffer_Initialize(&buffer, data, sizeof(data)));
}

TEST(Buffer, Clear)
{
    a_Buffer_t buffer;
    std::uint8_t data[1U];

    a_Buffer_Initialize(&buffer, data, sizeof(data));
    ASSERT_EQ(A_ERROR_NULL, a_Buffer_Clear(NULL));
    ASSERT_EQ(A_ERROR_NONE, a_Buffer_Clear(&buffer));
}

TEST(Buffer, SetWrite)
{
    a_Buffer_t buffer;
    std::uint8_t data[4U];

    a_Buffer_Initialize(&buffer, data, sizeof(data));
    ASSERT_EQ(A_ERROR_NULL, a_Buffer_SetWrite(NULL, 0U));
    ASSERT_EQ(A_ERROR_NONE, a_Buffer_SetWrite(&buffer, 0U));
    ASSERT_EQ(A_ERROR_NONE, a_Buffer_SetWrite(&buffer, 1U));
    ASSERT_EQ(A_ERROR_NONE, a_Buffer_SetWrite(&buffer, 2U));
    ASSERT_EQ(A_ERROR_NONE, a_Buffer_SetWrite(&buffer, 1U));
    ASSERT_EQ(A_ERROR_SIZE, a_Buffer_SetWrite(&buffer, 1U));

    a_Buffer_Clear(&buffer);
    ASSERT_EQ(A_ERROR_NONE, a_Buffer_SetWrite(&buffer, 4U));
    ASSERT_EQ(A_ERROR_SIZE, a_Buffer_SetWrite(&buffer, 1U));
}

TEST(Buffer, SetRead)
{
    a_Buffer_t buffer;
    std::uint8_t data[4U];

    a_Buffer_Initialize(&buffer, data, sizeof(data));
    a_Buffer_SetWrite(&buffer, 4U);
    ASSERT_EQ(A_ERROR_NULL, a_Buffer_SetRead(NULL, 0U));
    ASSERT_EQ(A_ERROR_NONE, a_Buffer_SetRead(&buffer, 0U));
    ASSERT_EQ(A_ERROR_NONE, a_Buffer_SetRead(&buffer, 1U));
    ASSERT_EQ(A_ERROR_NONE, a_Buffer_SetRead(&buffer, 2U));
    ASSERT_EQ(A_ERROR_NONE, a_Buffer_SetRead(&buffer, 1U));
    ASSERT_EQ(A_ERROR_NONE, a_Buffer_SetRead(&buffer, 0U));
    ASSERT_EQ(A_ERROR_SIZE, a_Buffer_SetRead(&buffer, 1U));

    a_Buffer_SetWrite(&buffer, 4U);
    ASSERT_EQ(A_ERROR_NONE, a_Buffer_SetRead(&buffer, 4U));
    ASSERT_EQ(A_ERROR_SIZE, a_Buffer_SetRead(&buffer, 1U));

    a_Buffer_SetWrite(&buffer, 4U);
    ASSERT_EQ(A_ERROR_SIZE, a_Buffer_SetRead(&buffer, 5U));
}