#include <cstdint>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "mock_socket.h"

#include "aether.h"

#define AETHER_TEST_BUFFER_SIZE 256U

class Subscriber
{
public:
    virtual void Callback(const char *const key, const std::uint8_t *const data, const std::size_t size, void *arg) const = 0;
};

class MockSubscriber : public Subscriber
{
public:
    MOCK_METHOD(void, Callback, (const char *const key, const std::uint8_t *const data, const std::size_t size, void *arg), (const, override));
};

class Aether : public testing::Test
{
protected:
    static std::size_t Send(const std::uint8_t *const data, const std::size_t size, void *arg)
    {
        return mock_socket_->Send(data, size, arg);
    }

    static std::size_t Receive(std::uint8_t *const data, const std::size_t size, void *arg)
    {
        return mock_socket_->Receive(data, size, arg);
    }

    static void Callback(const char *const key, const std::uint8_t *const data, const std::size_t size, void *arg)
    {
        return mock_subscriber_->Callback(key, data, size, arg);
    }

    void SetUp() override
    {
        mock_socket_ = new MockSocket;
        mock_subscriber_ = new MockSubscriber;
        a_Socket_Initialize(&socket_, A_SOCKET_TYPE_SERIAL, (a_Socket_Functions_t){.start = NULL, .stop = NULL, .send = Send, .receive = Receive}, send_buffer_, sizeof(send_buffer_), receive_buffer_, sizeof(receive_buffer_));
    }

    void TearDown() override
    {
        a_Deinitialize();
        delete mock_socket_;
        delete mock_subscriber_;
    }

    static MockSocket *mock_socket_;
    static MockSubscriber *mock_subscriber_;
    a_Socket_t socket_;
    std::uint8_t send_buffer_[AETHER_TEST_BUFFER_SIZE];
    std::uint8_t receive_buffer_[AETHER_TEST_BUFFER_SIZE];
    std::uint8_t message_buffer_[AETHER_TRANSPORT_MTU];
};

MockSocket *Aether::mock_socket_ = nullptr;
MockSubscriber *Aether::mock_subscriber_ = nullptr;

TEST_F(Aether, Initialize)
{
    ASSERT_EQ(A_ERR_NONE, a_Initialize(A_TRANSPORT_PEER_ID_MAX));
}

TEST_F(Aether, Deinitialize)
{
    a_Deinitialize();

    a_Initialize(A_TRANSPORT_PEER_ID_MAX);
    a_Deinitialize();
}

TEST_F(Aether, AddSocket)
{
    a_Initialize(A_TRANSPORT_PEER_ID_MAX);

    ASSERT_EQ(A_ERR_NULL, a_AddSocket(nullptr, message_buffer_, sizeof(message_buffer_), true));
    ASSERT_EQ(A_ERR_NULL, a_AddSocket(&socket_, nullptr, sizeof(message_buffer_), true));
    ASSERT_EQ(A_ERR_NONE, a_AddSocket(&socket_, message_buffer_, sizeof(message_buffer_), true));
}

TEST_F(Aether, Task)
{
    // TODO ensure random peer ID, current peer id = 12345678U
    std::uint8_t connect_message_invalid_lease[] = {0x02U, 0x01U, 0x05U, 0xCEU, 0xC2U, 0xF1U, 0x05U, 0x0DU, 0x80U, 0x10U, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0x01U, 0x00U};
    std::uint8_t connect_message_invalid_mtu[] = {0x02U, 0x01U, 0x0AU, 0xCEU, 0xC2U, 0xF1U, 0x05U, 0x01U, 0xFFU, 0xFFU, 0xE8U, 0x07U, 0x00U};
    std::uint8_t connect_message[] = {0x02U, 0x01U, 0x0AU, 0xCEU, 0xC2U, 0xF1U, 0x05U, 0x02U, 0x80U, 0x10U, 0xE8U, 0x07U, 0x00U};
    std::uint8_t accept_message[] = {0x0CU, 0x01U, 0x01U, 0xCEU, 0xC2U, 0xF1U, 0x05U, 0x03U, 0x80U, 0x08U, 0xFAU, 0x01U, 0x00U};
    std::uint8_t renew_message[] = {0x08U, 0x01U, 0x03U, 0xCEU, 0xC2U, 0xF1U, 0x05U, 0x04U, 0x00U};
    std::uint8_t subscribe_message_first[] = {0x0DU, 0x01U, 0x05U, 0xCEU, 0xC2U, 0xF1U, 0x05U, 0x05U, 0x05U, 0x2FU, 0x62U, 0x61U, 0x7AU, 0x01U, 0x00U};
    std::uint8_t subscribe_message_second[] = {0x0DU, 0x01U, 0x05U, 0xCEU, 0xC2U, 0xF1U, 0x05U, 0x06U, 0x05U, 0x2FU, 0x62U, 0x61U, 0x7AU, 0x01U, 0x00U};
    std::uint8_t subscribe_message_third[] = {0x0DU, 0x01U, 0x05U, 0xCEU, 0xC2U, 0xF1U, 0x05U, 0x07U, 0x05U, 0x2FU, 0x71U, 0x75U, 0x78U, 0x01U, 0x00U};
    std::uint8_t publish_message[] = {0x11U, 0x01U, 0x04U, 0xCEU, 0xC2U, 0xF1U, 0x05U, 0x08U, 0xF8U, 0xABU, 0xE2U, 0xE3U, 0x17U, 0x01U, 0x02U, 0x03U, 0x04U, 0x00U};
    std::uint8_t close_message[] = {0x08U, 0x01U, 0x02U, 0xCEU, 0xC2U, 0xF1U, 0x05U, 0x09U, 0x00U};
    std::uint8_t data[] = {0x01U, 0x02U, 0x03U, 0x04U};
    a_Initialize(A_TRANSPORT_PEER_ID_MAX);
    a_AddSocket(&socket_, message_buffer_, sizeof(message_buffer_), true);

    {
        testing::InSequence sequence;

        EXPECT_CALL(*mock_socket_, Send(testing::_, testing::_, testing::_)).Times(1).WillOnce(testing::ReturnArg<1>());
        for (std::size_t i = 0U; i < sizeof(connect_message_invalid_lease); i++)
        {
            EXPECT_CALL(*mock_socket_, Receive(testing::_, 1U, testing::_)).Times(1).WillOnce(testing::DoAll(testing::SetArgPointee<0>(connect_message_invalid_lease[i]), testing::Return(1U)));
        }
        EXPECT_CALL(*mock_socket_, Send(testing::_, testing::_, testing::_)).Times(1).WillOnce(testing::ReturnArg<1>());
        for (std::size_t i = 0U; i < sizeof(connect_message_invalid_mtu); i++)
        {
            EXPECT_CALL(*mock_socket_, Receive(testing::_, 1U, testing::_)).Times(1).WillOnce(testing::DoAll(testing::SetArgPointee<0>(connect_message_invalid_mtu[i]), testing::Return(1U)));
        }
        EXPECT_CALL(*mock_socket_, Send(testing::_, testing::_, testing::_)).Times(1).WillOnce(testing::ReturnArg<1>());
        for (std::size_t i = 0U; i < sizeof(connect_message); i++)
        {
            EXPECT_CALL(*mock_socket_, Receive(testing::_, 1U, testing::_)).Times(1).WillOnce(testing::DoAll(testing::SetArgPointee<0>(connect_message[i]), testing::Return(1U)));
        }
        EXPECT_CALL(*mock_socket_, Send(testing::_, testing::_, testing::_)).Times(1).WillOnce(testing::ReturnArg<1>());
        for (std::size_t i = 0U; i < sizeof(accept_message); i++)
        {
            EXPECT_CALL(*mock_socket_, Receive(testing::_, 1U, testing::_)).Times(1).WillOnce(testing::DoAll(testing::SetArgPointee<0>(accept_message[i]), testing::Return(1U)));
        }
        EXPECT_CALL(*mock_socket_, Send(testing::_, testing::_, testing::_)).Times(1).WillOnce(testing::ReturnArg<1>());
        EXPECT_CALL(*mock_socket_, Receive(testing::_, 1U, testing::_)).Times(1).WillOnce(testing::Return(0U));
        EXPECT_CALL(*mock_socket_, Send(testing::_, testing::_, testing::_)).Times(1).WillOnce(testing::ReturnArg<1>());
        EXPECT_CALL(*mock_socket_, Receive(testing::_, 1U, testing::_)).Times(1).WillOnce(testing::Return(0U));
        for (std::size_t i = 0U; i < sizeof(renew_message); i++)
        {
            EXPECT_CALL(*mock_socket_, Receive(testing::_, 1U, testing::_)).Times(1).WillOnce(testing::DoAll(testing::SetArgPointee<0>(renew_message[i]), testing::Return(1U)));
        }
        for (std::size_t i = 0U; i < sizeof(subscribe_message_first); i++)
        {
            EXPECT_CALL(*mock_socket_, Receive(testing::_, 1U, testing::_)).Times(1).WillOnce(testing::DoAll(testing::SetArgPointee<0>(subscribe_message_first[i]), testing::Return(1U)));
        }
        for (std::size_t i = 0U; i < sizeof(subscribe_message_second); i++)
        {
            EXPECT_CALL(*mock_socket_, Receive(testing::_, 1U, testing::_)).Times(1).WillOnce(testing::DoAll(testing::SetArgPointee<0>(subscribe_message_second[i]), testing::Return(1U)));
        }
        EXPECT_CALL(*mock_socket_, Send(testing::_, testing::_, testing::_)).Times(1).WillOnce(testing::ReturnArg<1>());
        for (std::size_t i = 0U; i < sizeof(subscribe_message_third); i++)
        {
            EXPECT_CALL(*mock_socket_, Receive(testing::_, 1U, testing::_)).Times(1).WillOnce(testing::DoAll(testing::SetArgPointee<0>(subscribe_message_third[i]), testing::Return(1U)));
        }
        EXPECT_CALL(*mock_socket_, Send(testing::_, testing::_, testing::_)).Times(1).WillOnce(testing::ReturnArg<1>());
        for (std::size_t i = 0U; i < sizeof(publish_message); i++)
        {
            EXPECT_CALL(*mock_socket_, Receive(testing::_, 1U, testing::_)).Times(1).WillOnce(testing::DoAll(testing::SetArgPointee<0>(publish_message[i]), testing::Return(1U)));
        }
        EXPECT_CALL(*mock_subscriber_, Callback(testing::StrEq("/foo"), testing::_, testing::_, nullptr)).With(testing::Args<1, 2>(testing::ElementsAreArray(data, sizeof(data)))).Times(1);
        EXPECT_CALL(*mock_socket_, Send(testing::_, testing::_, testing::_)).Times(1).WillOnce(testing::ReturnArg<1>());
        for (std::size_t i = 0U; i < sizeof(publish_message); i++)
        {
            EXPECT_CALL(*mock_socket_, Receive(testing::_, 1U, testing::_)).Times(1).WillOnce(testing::DoAll(testing::SetArgPointee<0>(publish_message[i]), testing::Return(1U)));
        }
        for (std::size_t i = 0U; i < sizeof(close_message); i++)
        {
            EXPECT_CALL(*mock_socket_, Receive(testing::_, 1U, testing::_)).Times(1).WillOnce(testing::DoAll(testing::SetArgPointee<0>(close_message[i]), testing::Return(1U)));
        }
    }

    a_Task(); // Send connect
    a_Task(); // Receive connect with invalid lease
    a_Task(); // Failed
    a_Task(); // Closed

    a_Task(); // Send connect
    a_Task(); // Receive connect with invalid MTU
    a_Task(); // Failed
    a_Task(); // Closed

    ASSERT_EQ(A_ERR_NONE, a_Subscribe("/foo", Callback, nullptr));

    a_Task(); // Send connect
    a_Task(); // Receive connect
    a_Task(); // Send accept

    ASSERT_EQ(A_ERR_NONE, a_Subscribe("/bar", Callback, nullptr));

    a_Task(); // Do nothing
    a_Task(); // Receive renew

    a_EnableRouting(false);
    a_Task(); // Receive subscribe first
    ASSERT_EQ(A_ERR_NONE, a_Publish("/baz", data, sizeof(data)));
    a_Declare("/baz");
    a_Task(); // Receive subscribe second
    ASSERT_EQ(A_ERR_NONE, a_Publish("/baz", data, sizeof(data)));

    a_EnableRouting(true);
    a_Task(); // Receive subscribe third
    ASSERT_EQ(A_ERR_NONE, a_Publish("/qux", data, sizeof(data)));

    a_Task(); // Receive publish

    ASSERT_EQ(A_ERR_NONE, a_Unsubscribe("/foo"));
    a_Task(); // Receive publish

    a_Task(); // Receive close
    a_Task(); // Close session
}

TEST_F(Aether, Declare)
{
    a_Initialize(A_TRANSPORT_PEER_ID_MAX);
    a_AddSocket(&socket_, message_buffer_, sizeof(message_buffer_), true);

    ASSERT_EQ(A_ERR_NULL, a_Declare(nullptr));
}

TEST_F(Aether, Publish)
{
    std::uint8_t data[] = {0x01U, 0x02U, 0x03U, 0x04U};
    a_Initialize(A_TRANSPORT_PEER_ID_MAX);
    a_AddSocket(&socket_, message_buffer_, sizeof(message_buffer_), true);

    ASSERT_EQ(A_ERR_NULL, a_Publish(nullptr, data, sizeof(data)));
    ASSERT_EQ(A_ERR_NULL, a_Publish("/foo", nullptr, sizeof(data)));

    ASSERT_EQ(A_ERR_SIZE, a_Publish("/foo", data, 0U));

    ASSERT_EQ(A_ERR_NONE, a_Publish("/foo", data, sizeof(data)));
}

TEST_F(Aether, Subscribe)
{
    a_Initialize(A_TRANSPORT_PEER_ID_MAX);
    a_AddSocket(&socket_, message_buffer_, sizeof(message_buffer_), true);

    ASSERT_EQ(A_ERR_NULL, a_Subscribe(nullptr, Callback, nullptr));
    ASSERT_EQ(A_ERR_NULL, a_Subscribe("/foo", nullptr, nullptr));

    ASSERT_EQ(A_ERR_NONE, a_Subscribe("/foo", Callback, nullptr));
}

TEST_F(Aether, Unsubscribe)
{
    a_Initialize(A_TRANSPORT_PEER_ID_MAX);
    a_AddSocket(&socket_, message_buffer_, sizeof(message_buffer_), true);

    ASSERT_EQ(A_ERR_NULL, a_Unsubscribe(nullptr));

    ASSERT_EQ(A_ERR_NONE, a_Unsubscribe("/foo"));
}
