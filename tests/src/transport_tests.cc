#include <cstdint>

#include <gtest/gtest.h>

#include "err.h"
#include "transport.h"

TEST(Transport, MessageInitialize)
{
    a_Transport_Message_t message;
    std::uint8_t buffer[AETHER_TRANSPORT_MTU];

    ASSERT_EQ(A_ERR_NULL, a_Transport_MessageInitialize(nullptr, buffer, sizeof(buffer)));
    ASSERT_EQ(A_ERR_NULL, a_Transport_MessageInitialize(&message, nullptr, sizeof(buffer)));

    ASSERT_EQ(A_ERR_SIZE, a_Transport_MessageInitialize(&message, buffer, sizeof(buffer) - 1U));

    ASSERT_EQ(A_ERR_NONE, a_Transport_MessageInitialize(&message, buffer, sizeof(buffer)));
}

TEST(Transport, MessageConnect)
{
    a_Transport_Message_t message;
    std::uint8_t buffer[AETHER_TRANSPORT_MTU];
    a_Transport_MessageInitialize(&message, buffer, sizeof(buffer));

    ASSERT_EQ(A_ERR_NULL, a_Transport_MessageConnect(nullptr, 1000U));

    ASSERT_EQ(A_ERR_NONE, a_Transport_MessageConnect(&message, 1000U));
}

TEST(Transport, MessageAccept)
{
    a_Transport_Message_t message;
    std::uint8_t buffer[AETHER_TRANSPORT_MTU];
    a_Transport_MessageInitialize(&message, buffer, sizeof(buffer));

    ASSERT_EQ(A_ERR_NULL, a_Transport_MessageAccept(nullptr, 1000U));

    ASSERT_EQ(A_ERR_NONE, a_Transport_MessageAccept(&message, 1000U));
}

TEST(Transport, MessageClose)
{
    a_Transport_Message_t message;
    std::uint8_t buffer[AETHER_TRANSPORT_MTU];
    a_Transport_MessageInitialize(&message, buffer, sizeof(buffer));

    ASSERT_EQ(A_ERR_NULL, a_Transport_MessageClose(nullptr));

    ASSERT_EQ(A_ERR_NONE, a_Transport_MessageClose(&message));
}

TEST(Transport, MessageRenew)
{
    a_Transport_Message_t message;
    std::uint8_t buffer[AETHER_TRANSPORT_MTU];
    a_Transport_MessageInitialize(&message, buffer, sizeof(buffer));

    ASSERT_EQ(A_ERR_NULL, a_Transport_MessageRenew(nullptr));

    ASSERT_EQ(A_ERR_NONE, a_Transport_MessageRenew(&message));
}

TEST(Transport, SerializeMessage)
{
    a_Transport_Message_t message;
    std::uint8_t buffer[AETHER_TRANSPORT_MTU];
    a_Transport_MessageInitialize(&message, buffer, sizeof(buffer));
    a_Transport_MessageConnect(&message, 1000U);

    ASSERT_EQ(A_ERR_NULL, a_Transport_SerializeMessage(nullptr, 0x1234U, 0x5678U));

    ASSERT_EQ(A_ERR_NONE, a_Transport_SerializeMessage(&message, 0x1234U, 0x5678U));
    /* TODO verify buffer */
}

TEST(Transport, DeserializeMessage)
{
    a_Transport_Message_t message;
    std::uint8_t buffer[AETHER_TRANSPORT_MTU];
    a_Transport_MessageInitialize(&message, buffer, sizeof(buffer));

    ASSERT_EQ(A_ERR_NULL, a_Transport_DeserializeMessage(nullptr));

    ASSERT_EQ(A_ERR_SERIALIZATION, a_Transport_DeserializeMessage(&message));

    a_Transport_MessageConnect(&message, 1000U);
    a_Transport_SerializeMessage(&message, 0x1234U, 0x5678U);
    ASSERT_EQ(A_ERR_NONE, a_Transport_DeserializeMessage(&message));
    /* TODO verify fields */
}

TEST(Transport, IsMessageSerialized)
{
    a_Transport_Message_t message;
    std::uint8_t buffer[AETHER_TRANSPORT_MTU];
    a_Transport_MessageInitialize(&message, buffer, sizeof(buffer));
    a_Transport_MessageConnect(&message, 1000U);

    ASSERT_FALSE(a_Transport_IsMessageSerialized(nullptr));
    ASSERT_FALSE(a_Transport_IsMessageSerialized(&message));

    a_Transport_SerializeMessage(&message, 0x1234U, 0x5678U);
    ASSERT_TRUE(a_Transport_IsMessageSerialized(&message));
}

TEST(Transport, IsMessageDeserialized)
{
    a_Transport_Message_t message;
    std::uint8_t buffer[AETHER_TRANSPORT_MTU];
    a_Transport_MessageInitialize(&message, buffer, sizeof(buffer));
    a_Transport_MessageConnect(&message, 1000U);
    a_Transport_SerializeMessage(&message, 0x1234U, 0x5678U);

    ASSERT_FALSE(a_Transport_IsMessageDeserialized(nullptr));
    ASSERT_FALSE(a_Transport_IsMessageDeserialized(&message));

    a_Transport_DeserializeMessage(&message);
    ASSERT_TRUE(a_Transport_IsMessageDeserialized(&message));
}

TEST(Transport, GetMessageBuffer)
{
    a_Transport_Message_t message;
    std::uint8_t buffer[AETHER_TRANSPORT_MTU];
    a_Transport_MessageInitialize(&message, buffer, sizeof(buffer));

    ASSERT_EQ(nullptr, a_Transport_GetMessageBuffer(nullptr));

    ASSERT_NE(nullptr, a_Transport_GetMessageBuffer(&message));
}

TEST(Transport, GetMessageHeader)
{
    a_Transport_Message_t message;
    std::uint8_t buffer[AETHER_TRANSPORT_MTU];
    a_Transport_MessageInitialize(&message, buffer, sizeof(buffer));

    ASSERT_EQ(A_TRANSPORT_HEADER_MAX, a_Transport_GetMessageHeader(nullptr));
    ASSERT_EQ(A_TRANSPORT_HEADER_MAX, a_Transport_GetMessageHeader(&message));

    a_Transport_MessageConnect(&message, 1000U);
    a_Transport_SerializeMessage(&message, A_TRANSPORT_PEER_ID_MAX - 1U, A_TRANSPORT_SEQUENCE_NUMBER_MAX - 1U);
    a_Transport_DeserializeMessage(&message);
    ASSERT_EQ(A_TRANSPORT_HEADER_CONNECT, a_Transport_GetMessageHeader(&message));
}

TEST(Transport, GetMessagePeerId)
{
    a_Transport_Message_t message;
    std::uint8_t buffer[AETHER_TRANSPORT_MTU];
    a_Transport_MessageInitialize(&message, buffer, sizeof(buffer));

    ASSERT_EQ(A_TRANSPORT_PEER_ID_MAX, a_Transport_GetMessagePeerId(nullptr));
    ASSERT_EQ(A_TRANSPORT_PEER_ID_MAX, a_Transport_GetMessagePeerId(&message));

    a_Transport_MessageConnect(&message, 1000U);
    a_Transport_SerializeMessage(&message, A_TRANSPORT_PEER_ID_MAX - 1U, A_TRANSPORT_SEQUENCE_NUMBER_MAX - 1U);
    a_Transport_DeserializeMessage(&message);
    ASSERT_EQ(A_TRANSPORT_PEER_ID_MAX - 1U, a_Transport_GetMessagePeerId(&message));
}

TEST(Transport, GetMessageSequenceNumber)
{
    a_Transport_Message_t message;
    std::uint8_t buffer[AETHER_TRANSPORT_MTU];
    a_Transport_MessageInitialize(&message, buffer, sizeof(buffer));

    ASSERT_EQ(A_TRANSPORT_SEQUENCE_NUMBER_MAX, a_Transport_GetMessageSequenceNumber(nullptr));
    ASSERT_EQ(A_TRANSPORT_SEQUENCE_NUMBER_MAX, a_Transport_GetMessageSequenceNumber(&message));

    a_Transport_MessageConnect(&message, 1000U);
    a_Transport_SerializeMessage(&message, A_TRANSPORT_PEER_ID_MAX - 1U, A_TRANSPORT_SEQUENCE_NUMBER_MAX - 1U);
    a_Transport_DeserializeMessage(&message);
    ASSERT_EQ(A_TRANSPORT_SEQUENCE_NUMBER_MAX - 1U, a_Transport_GetMessageSequenceNumber(&message));
}

TEST(Transport, GetMessageLease)
{
    a_Transport_Message_t message;
    std::uint8_t buffer[AETHER_TRANSPORT_MTU];
    a_Transport_MessageInitialize(&message, buffer, sizeof(buffer));

    ASSERT_EQ(A_TICK_MS_MAX, a_Transport_GetMessageLease(nullptr));
    ASSERT_EQ(A_TICK_MS_MAX, a_Transport_GetMessageLease(&message));

    a_Transport_MessageConnect(&message, 1000U);
    a_Transport_SerializeMessage(&message, A_TRANSPORT_PEER_ID_MAX - 1U, A_TRANSPORT_SEQUENCE_NUMBER_MAX - 1U);
    a_Transport_DeserializeMessage(&message);
    ASSERT_EQ(1000U, a_Transport_GetMessageLease(&message));

    a_Transport_MessageInitialize(&message, buffer, sizeof(buffer));
    a_Transport_MessageAccept(&message, 5000U);
    a_Transport_SerializeMessage(&message, A_TRANSPORT_PEER_ID_MAX - 1U, A_TRANSPORT_SEQUENCE_NUMBER_MAX - 1U);
    a_Transport_DeserializeMessage(&message);
    ASSERT_EQ(5000U, a_Transport_GetMessageLease(&message));
}
