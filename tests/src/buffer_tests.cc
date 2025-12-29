#include <cstdint>

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include "buffer.h"
#include "err.h"

TEST(Buffer, Initialize)
{
    a_Buffer_t buffer;
    std::uint8_t data[1U];

    ASSERT_EQ(A_ERR_NULL, a_Buffer_Initialize(nullptr, data, sizeof(data)));
    ASSERT_EQ(A_ERR_NULL, a_Buffer_Initialize(&buffer, nullptr, sizeof(data)));
    ASSERT_EQ(A_ERR_NULL, a_Buffer_Initialize(nullptr, nullptr, sizeof(data)));
    ASSERT_EQ(A_ERR_NONE, a_Buffer_Initialize(&buffer, data, 0U));
    ASSERT_EQ(A_ERR_NONE, a_Buffer_Initialize(&buffer, data, sizeof(data)));
}

TEST(Buffer, Clear)
{
    a_Buffer_t buffer;
    std::uint8_t data[1U];

    a_Buffer_Initialize(&buffer, data, sizeof(data));
    ASSERT_EQ(A_ERR_NULL, a_Buffer_Clear(nullptr));
    ASSERT_EQ(A_ERR_NONE, a_Buffer_Clear(&buffer));
}

TEST(Buffer, SetWrite)
{
    a_Buffer_t buffer;
    std::uint8_t data[4U];

    a_Buffer_Initialize(&buffer, data, sizeof(data));
    ASSERT_EQ(A_ERR_NULL, a_Buffer_SetWrite(nullptr, 0U));
    ASSERT_EQ(A_ERR_NONE, a_Buffer_SetWrite(&buffer, 0U));
    ASSERT_EQ(A_ERR_NONE, a_Buffer_SetWrite(&buffer, 1U));
    ASSERT_EQ(A_ERR_NONE, a_Buffer_SetWrite(&buffer, 2U));
    ASSERT_EQ(A_ERR_NONE, a_Buffer_SetWrite(&buffer, 1U));
    ASSERT_EQ(A_ERR_SIZE, a_Buffer_SetWrite(&buffer, 1U));

    a_Buffer_Clear(&buffer);
    ASSERT_EQ(A_ERR_NONE, a_Buffer_SetWrite(&buffer, 4U));
    ASSERT_EQ(A_ERR_SIZE, a_Buffer_SetWrite(&buffer, 1U));
}

TEST(Buffer, SetRead)
{
    a_Buffer_t buffer;
    std::uint8_t data[4U];

    a_Buffer_Initialize(&buffer, data, sizeof(data));
    a_Buffer_SetWrite(&buffer, 4U);
    ASSERT_EQ(A_ERR_NULL, a_Buffer_SetRead(nullptr, 0U));
    ASSERT_EQ(A_ERR_NONE, a_Buffer_SetRead(&buffer, 0U));
    ASSERT_EQ(A_ERR_NONE, a_Buffer_SetRead(&buffer, 1U));
    ASSERT_EQ(A_ERR_NONE, a_Buffer_SetRead(&buffer, 2U));
    ASSERT_EQ(A_ERR_NONE, a_Buffer_SetRead(&buffer, 1U));
    ASSERT_EQ(A_ERR_NONE, a_Buffer_SetRead(&buffer, 0U));
    ASSERT_EQ(A_ERR_SIZE, a_Buffer_SetRead(&buffer, 1U));

    a_Buffer_SetWrite(&buffer, 4U);
    ASSERT_EQ(A_ERR_NONE, a_Buffer_SetRead(&buffer, 4U));
    ASSERT_EQ(A_ERR_SIZE, a_Buffer_SetRead(&buffer, 1U));

    a_Buffer_Clear(&buffer);
    a_Buffer_SetWrite(&buffer, 4U);
    ASSERT_EQ(A_ERR_SIZE, a_Buffer_SetRead(&buffer, 5U));
}

TEST(Buffer, GetWrite)
{
    a_Buffer_t buffer;
    std::uint8_t data[4U];

    a_Buffer_Initialize(&buffer, data, sizeof(data));
    ASSERT_EQ(nullptr, a_Buffer_GetWrite(nullptr));
    ASSERT_EQ(&data[0U], a_Buffer_GetWrite(&buffer));

    a_Buffer_SetWrite(&buffer, 1U);
    ASSERT_EQ(&data[1U], a_Buffer_GetWrite(&buffer));

    a_Buffer_SetWrite(&buffer, 2U);
    ASSERT_EQ(&data[3U], a_Buffer_GetWrite(&buffer));

    a_Buffer_SetWrite(&buffer, 1U);
    ASSERT_EQ(nullptr, a_Buffer_GetWrite(&buffer));
}

TEST(Buffer, GetRead)
{
    a_Buffer_t buffer;
    std::uint8_t data[4U];

    a_Buffer_Initialize(&buffer, data, sizeof(data));
    ASSERT_EQ(nullptr, a_Buffer_GetRead(nullptr));
    ASSERT_EQ(nullptr, a_Buffer_GetRead(&buffer));

    a_Buffer_SetWrite(&buffer, 1U);
    ASSERT_EQ(&data[0U], a_Buffer_GetRead(&buffer));
    a_Buffer_SetRead(&buffer, 1U);
    ASSERT_EQ(nullptr, a_Buffer_GetRead(&buffer));

    a_Buffer_SetWrite(&buffer, 2U);
    ASSERT_EQ(&data[0U], a_Buffer_GetRead(&buffer));
    a_Buffer_SetRead(&buffer, 1U);
    ASSERT_EQ(&data[1U], a_Buffer_GetRead(&buffer));
    a_Buffer_SetRead(&buffer, 1U);
    ASSERT_EQ(nullptr, a_Buffer_GetRead(&buffer));

    a_Buffer_SetWrite(&buffer, 4U);
    a_Buffer_SetRead(&buffer, 3U);
    ASSERT_EQ(&data[3U], a_Buffer_GetRead(&buffer));
    a_Buffer_SetRead(&buffer, 1U);
    ASSERT_EQ(nullptr, a_Buffer_GetRead(&buffer));
}

TEST(Buffer, GetWriteSize)
{
    a_Buffer_t buffer;
    std::uint8_t data[4U];

    a_Buffer_Initialize(&buffer, data, sizeof(data));
    ASSERT_EQ(0U, a_Buffer_GetWriteSize(nullptr));
    ASSERT_EQ(4U, a_Buffer_GetWriteSize(&buffer));

    a_Buffer_SetWrite(&buffer, 2U);
    ASSERT_EQ(2U, a_Buffer_GetWriteSize(&buffer));

    a_Buffer_SetWrite(&buffer, 1U);
    ASSERT_EQ(1U, a_Buffer_GetWriteSize(&buffer));

    a_Buffer_SetWrite(&buffer, 1U);
    ASSERT_EQ(0U, a_Buffer_GetWriteSize(&buffer));
}

TEST(Buffer, GetReadSize)
{
    a_Buffer_t buffer;
    std::uint8_t data[4U];

    a_Buffer_Initialize(&buffer, data, sizeof(data));
    ASSERT_EQ(0U, a_Buffer_GetReadSize(nullptr));
    ASSERT_EQ(0U, a_Buffer_GetReadSize(&buffer));

    a_Buffer_SetWrite(&buffer, 4U);
    ASSERT_EQ(4U, a_Buffer_GetReadSize(&buffer));

    a_Buffer_SetRead(&buffer, 2U);
    ASSERT_EQ(2U, a_Buffer_GetReadSize(&buffer));

    a_Buffer_SetRead(&buffer, 1U);
    ASSERT_EQ(1U, a_Buffer_GetReadSize(&buffer));

    a_Buffer_SetRead(&buffer, 1U);
    ASSERT_EQ(0U, a_Buffer_GetReadSize(&buffer));
}

TEST(Buffer, AppendLeft)
{
    a_Buffer_t buffer;
    a_Buffer_t append;
    std::uint8_t data_buffer[8U] = {0x00U};
    std::uint8_t data_append[4U] = {0x01U, 0x02U, 0x03U, 0x04U};
    std::uint8_t *read = nullptr;

    a_Buffer_Initialize(&buffer, data_buffer, sizeof(data_buffer));
    a_Buffer_Initialize(&append, data_append, sizeof(data_append));
    ASSERT_EQ(A_ERR_NULL, a_Buffer_AppendLeft(nullptr, &append));
    ASSERT_EQ(A_ERR_NULL, a_Buffer_AppendLeft(&buffer, nullptr));
    ASSERT_EQ(A_ERR_NULL, a_Buffer_AppendLeft(nullptr, nullptr));

    ASSERT_EQ(A_ERR_NONE, a_Buffer_AppendLeft(&buffer, &append));
    ASSERT_EQ(0U, a_Buffer_GetReadSize(&buffer));

    a_Buffer_SetWrite(&append, 2U);
    ASSERT_EQ(A_ERR_NONE, a_Buffer_AppendLeft(&buffer, &append));
    ASSERT_EQ(2U, a_Buffer_GetReadSize(&buffer));
    read = a_Buffer_GetRead(&buffer);
    ASSERT_EQ(0x01, *read);
    ASSERT_EQ(0x02, *(read + 1U));

    a_Buffer_SetRead(&buffer, 1U);
    a_Buffer_SetWrite(&append, 1U);
    ASSERT_EQ(A_ERR_NONE, a_Buffer_AppendLeft(&buffer, &append));
    ASSERT_EQ(4U, a_Buffer_GetReadSize(&buffer));
    read = a_Buffer_GetRead(&buffer);
    ASSERT_EQ(0x01, *read);
    ASSERT_EQ(0x02, *(read + 1U));
    ASSERT_EQ(0x03, *(read + 2U));
    ASSERT_EQ(0x02, *(read + 3U));

    a_Buffer_SetWrite(&append, 1U);
    ASSERT_EQ(A_ERR_SIZE, a_Buffer_AppendLeft(&buffer, &append));
}

TEST(Buffer, AppendRight)
{
    a_Buffer_t buffer;
    a_Buffer_t append;
    std::uint8_t data_buffer[8U] = {0x00U};
    std::uint8_t data_append[4U] = {0x01U, 0x02U, 0x03U, 0x04U};
    std::uint8_t *read = nullptr;

    a_Buffer_Initialize(&buffer, data_buffer, sizeof(data_buffer));
    a_Buffer_Initialize(&append, data_append, sizeof(data_append));
    ASSERT_EQ(A_ERR_NULL, a_Buffer_AppendRight(nullptr, &append));
    ASSERT_EQ(A_ERR_NULL, a_Buffer_AppendRight(&buffer, nullptr));
    ASSERT_EQ(A_ERR_NULL, a_Buffer_AppendRight(nullptr, nullptr));

    ASSERT_EQ(A_ERR_NONE, a_Buffer_AppendRight(&buffer, &append));
    ASSERT_EQ(0U, a_Buffer_GetReadSize(&buffer));

    a_Buffer_SetWrite(&append, 2U);
    ASSERT_EQ(A_ERR_NONE, a_Buffer_AppendRight(&buffer, &append));
    ASSERT_EQ(2U, a_Buffer_GetReadSize(&buffer));
    read = a_Buffer_GetRead(&buffer);
    ASSERT_EQ(0x01, *read);
    ASSERT_EQ(0x02, *(read + 1U));

    a_Buffer_SetRead(&buffer, 1U);
    a_Buffer_SetWrite(&append, 1U);
    ASSERT_EQ(A_ERR_NONE, a_Buffer_AppendRight(&buffer, &append));
    ASSERT_EQ(4U, a_Buffer_GetReadSize(&buffer));
    read = a_Buffer_GetRead(&buffer);
    ASSERT_EQ(0x02, *read);
    ASSERT_EQ(0x01, *(read + 1U));
    ASSERT_EQ(0x02, *(read + 2U));
    ASSERT_EQ(0x03, *(read + 3U));

    a_Buffer_SetWrite(&append, 1U);
    ASSERT_EQ(A_ERR_SIZE, a_Buffer_AppendRight(&buffer, &append));
}
