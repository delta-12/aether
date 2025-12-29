#include <cstddef>
#include <cstdint>
#include <cstring>
#include <memory>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "buffer.h"
#include "err.h"
#include "serial.h"
#include "socket.h"

#define SERIAL_TEST_BUFFER_SIZE 256U

class Socket
{
public:
    virtual std::size_t Send(const std::uint8_t *const data, const std::size_t size) const = 0;
    virtual std::size_t Receive(uint8_t *const data, const size_t size) const = 0;
};

class MockSocket : public Socket
{
public:
    MOCK_METHOD(std::size_t, Send, (const std::uint8_t *const data, const std::size_t size), (const, override));
    MOCK_METHOD(std::size_t, Receive, (uint8_t *const data, const size_t size), (const, override));
};

class Serial : public testing::Test
{
protected:
    static std::size_t Send(const std::uint8_t *const data, const std::size_t size)
    {
        return mock_socket_->Send(data, size);
    }

    static std::size_t Receive(uint8_t *const data, const size_t size)
    {
        return mock_socket_->Receive(data, size);
    }

    void SetUp() override
    {
        mock_socket_ = new MockSocket;
        a_Socket_Initialize(&socket_, A_SOCKET_TYPE_SERIAL, Send, send_buffer, sizeof(send_buffer), Receive, receive_buffer, sizeof(receive_buffer));
    }

    void TearDown() override
    {
        delete mock_socket_;
    }

    static MockSocket *mock_socket_;
    a_Socket_t socket_;
    std::uint8_t send_buffer[SERIAL_TEST_BUFFER_SIZE];
    std::uint8_t receive_buffer[SERIAL_TEST_BUFFER_SIZE];
};

MockSocket *Serial::mock_socket_ = nullptr;

TEST_F(Serial, Send)
{
    std::uint8_t data[] = {0x01U, 0x02U, 0x00U, 0x03U, 0x04U, 0x05U};
    std::uint8_t encoded[] = {0x03U, 0x01U, 0x02U, 0x04U, 0x03U, 0x04U, 0x05U, 0x00U};
    a_Buffer_t buffer;

    {
        testing::InSequence sequence;

        EXPECT_CALL(*mock_socket_, Send(testing::_, testing::_)).Times(1).WillOnce(testing::Return(SIZE_MAX));
        EXPECT_CALL(*mock_socket_, Send(testing::Truly([&encoded](const std::uint8_t *const sent)
                                                       { return (0 == std::memcmp(sent, encoded, sizeof(encoded))) &&
                                                                (0 == std::memcmp((sent + sizeof(encoded)), encoded, sizeof(encoded))); }),
                                        sizeof(encoded) * 2U))
            .Times(2)
            .WillOnce(testing::Return(sizeof(encoded)))
            .WillOnce(testing::Return(sizeof(encoded) * 2U));
    }

    a_Buffer_Initialize(&buffer, data, sizeof(data));
    a_Buffer_SetWrite(&buffer, sizeof(data));

    ASSERT_EQ(A_ERR_NULL, a_Serial_Send(nullptr, &buffer));
    ASSERT_EQ(A_ERR_NULL, a_Serial_Send(&socket_, nullptr));

    ASSERT_EQ(A_ERR_SOCKET, a_Serial_Send(&socket_, &buffer));
    ASSERT_EQ(0U, a_Buffer_GetReadSize(&buffer));

    a_Buffer_SetWrite(&buffer, sizeof(data));
    ASSERT_EQ(A_ERR_NONE, a_Serial_Send(&socket_, &buffer));
    ASSERT_EQ(0U, a_Buffer_GetReadSize(&buffer));
    ASSERT_EQ(8U, a_Buffer_GetReadSize(&socket_.send_buffer));

    a_Buffer_SetWrite(&buffer, sizeof(data));
    ASSERT_EQ(A_ERR_NONE, a_Serial_Send(&socket_, &buffer));
    ASSERT_EQ(0U, a_Buffer_GetReadSize(&buffer));
    ASSERT_EQ(0U, a_Buffer_GetReadSize(&socket_.send_buffer));
}

TEST_F(Serial, Receive)
{
    a_Buffer_t data;

    ASSERT_EQ(A_ERR_NULL, a_Serial_Receive(nullptr, &data));
    ASSERT_EQ(A_ERR_NULL, a_Serial_Receive(&socket_, nullptr));

    /* TODO */
}