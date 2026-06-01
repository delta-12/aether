#include <cstdint>
#include <span>

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include "err.h"
#include "hash.h"
#include "leb128.h"
#include "transport.h"

#define SPAN_FROM_VALUE(value, size) std::span<std::uint8_t>(static_cast<std::uint8_t *>(value), size)
#define SERIALIZE_BUFFER_SIZE (LEB128_MAX_SIZE(a_Transport_Version_t) + LEB128_MAX_SIZE(uint64_t) + LEB128_MAX_SIZE(a_Transport_PeerId_t) + \
                               LEB128_MAX_SIZE(a_Transport_SequenceNumber_t))
#define BUFFER_SIZE_MIN (SERIALIZE_BUFFER_SIZE + LEB128_MAX_SIZE(a_Transport_Mtu_t) + LEB128_MAX_SIZE(a_Tick_Ms_t))

TEST(Transport, MessageInitialize)
{
    a_Transport_Message_t message;
    std::uint8_t buffer[AETHER_TRANSPORT_MTU];

    ASSERT_EQ(A_ERR_NULL, a_Transport_MessageInitialize(nullptr, buffer, sizeof(buffer)));
    ASSERT_EQ(A_ERR_NULL, a_Transport_MessageInitialize(&message, nullptr, sizeof(buffer)));

    ASSERT_EQ(A_ERR_SIZE, a_Transport_MessageInitialize(&message, buffer, BUFFER_SIZE_MIN - 1U));

    ASSERT_EQ(A_ERR_NONE, a_Transport_MessageInitialize(&message, buffer, sizeof(buffer) - 1U));
    ASSERT_EQ(A_ERR_NONE, a_Transport_MessageInitialize(&message, buffer, sizeof(buffer)));
    ASSERT_EQ(A_ERR_NONE, a_Transport_MessageInitialize(&message, buffer, sizeof(buffer) + 1U));
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

TEST(Transport, MessagePublish)
{
    a_Transport_Message_t message;
    std::uint8_t data[] = {0x00U, 0x01, 0x02, 0x03};
    std::uint8_t buffer[SERIALIZE_BUFFER_SIZE + LEB128_MAX_SIZE(uint64_t) + sizeof(data)];
    a_Transport_MessageInitialize(&message, buffer, sizeof(buffer));

    ASSERT_EQ(A_ERR_NULL, a_Transport_MessagePublish(nullptr, "/foo/bar", data, sizeof(data)));
    ASSERT_EQ(A_ERR_NULL, a_Transport_MessagePublish(&message, nullptr, data, sizeof(data)));
    ASSERT_EQ(A_ERR_NULL, a_Transport_MessagePublish(&message, "/foo/bar", nullptr, sizeof(data)));

    ASSERT_EQ(A_ERR_SIZE, a_Transport_MessagePublish(&message, "/foo/bar", data, 0U));
    ASSERT_EQ(A_ERR_SIZE, a_Transport_MessagePublish(&message, "/foo/bar", data, sizeof(data) + 1U));

    ASSERT_EQ(A_ERR_NONE, a_Transport_MessagePublish(&message, "/foo/bar", data, sizeof(data)));
}

TEST(Transport, MessageSubscribe)
{
    a_Transport_Message_t message;
    std::uint8_t buffer[SERIALIZE_BUFFER_SIZE + LEB128_MAX_SIZE(uint64_t) + 9U];
    a_Transport_MessageInitialize(&message, buffer, sizeof(buffer));

    ASSERT_EQ(A_ERR_NULL, a_Transport_MessageSubscribe(nullptr, "/foo/bar"));
    ASSERT_EQ(A_ERR_NULL, a_Transport_MessageSubscribe(&message, nullptr));

    ASSERT_EQ(A_ERR_SIZE, a_Transport_MessageSubscribe(&message, "/foo/bar/"));

    ASSERT_EQ(A_ERR_NONE, a_Transport_MessageSubscribe(&message, "/foo/bar"));
}

TEST(Transport, MessageUnsubscribe)
{
    a_Transport_Message_t message;
    std::uint8_t buffer[SERIALIZE_BUFFER_SIZE + LEB128_MAX_SIZE(uint64_t) + 9U];
    a_Transport_MessageInitialize(&message, buffer, sizeof(buffer));

    ASSERT_EQ(A_ERR_NULL, a_Transport_MessageUnsubscribe(nullptr, "/foo/bar"));
    ASSERT_EQ(A_ERR_NULL, a_Transport_MessageUnsubscribe(&message, nullptr));

    ASSERT_EQ(A_ERR_SIZE, a_Transport_MessageUnsubscribe(&message, "/foo/bar/"));

    ASSERT_EQ(A_ERR_NONE, a_Transport_MessageUnsubscribe(&message, "/foo/bar"));
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

TEST(Transport, CopyMessage)
{
    a_Transport_Message_t message;
    std::uint8_t message_buffer[AETHER_TRANSPORT_MTU];
    a_Transport_Message_t copy;
    std::uint8_t copy_buffer[AETHER_TRANSPORT_MTU];
    a_Transport_MessageInitialize(&message, message_buffer, sizeof(message_buffer));
    a_Transport_MessageInitialize(&copy, copy_buffer, sizeof(copy_buffer));

    ASSERT_EQ(A_ERR_NULL, a_Transport_CopyMessage(nullptr, &copy));
    ASSERT_EQ(A_ERR_NULL, a_Transport_CopyMessage(&message, nullptr));

    a_Transport_MessageConnect(&message, 1000U);
    a_Transport_SerializeMessage(&message, 0x1234U, 0x5678U);
    ASSERT_EQ(A_ERR_NONE, a_Transport_CopyMessage(&message, &copy));
    ASSERT_EQ(A_ERR_NONE, a_Transport_DeserializeMessage(&copy));
    /* TODO verify original message unchanged */
    /* TODO verify fields */
}

TEST(Transport, CopyString)
{
    const char *const string_0 = "foobar";
    const char *const string_1 = "fooba";
    char copy_0[strlen(string_0) + 1U] = {0};
    char copy_1[strlen(string_1) + 1U] = {0};

    a_Transport_CopyString(copy_0, string_0, strlen(string_0) + 1U);
    ASSERT_STREQ(copy_0, string_0);

    a_Transport_CopyString(copy_1, string_0, strlen(string_0));
    ASSERT_STREQ(copy_1, string_1);
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

TEST(Transport, GetStringSize)
{
    ASSERT_EQ(0U, a_Transport_GetStringSize(nullptr));
    ASSERT_EQ(4U, a_Transport_GetStringSize("foo"));
}

TEST(Transport, GetMessageBuffer)
{
    a_Transport_Message_t message;
    std::uint8_t buffer[AETHER_TRANSPORT_MTU];
    a_Transport_MessageInitialize(&message, buffer, sizeof(buffer));

    ASSERT_EQ(nullptr, a_Transport_GetBuffer(nullptr));

    ASSERT_NE(nullptr, a_Transport_GetBuffer(&message));
}

TEST(Transport, GetMtu)
{
    a_Transport_Message_t message;
    std::uint8_t buffer[AETHER_TRANSPORT_MTU];
    a_Transport_MessageInitialize(&message, buffer, sizeof(buffer));

    ASSERT_EQ(A_TRANSPORT_MTU_MAX, a_Transport_GetMtu(nullptr));

    ASSERT_EQ(sizeof(buffer), a_Transport_GetMtu(&message));
}

TEST(Transport, GetVersion)
{
    a_Transport_Message_t message;
    std::uint8_t buffer[AETHER_TRANSPORT_MTU];
    a_Transport_MessageInitialize(&message, buffer, sizeof(buffer));

    ASSERT_EQ(A_TRANSPORT_VERSION_MAX, a_Transport_GetMessageVersion(nullptr));
    ASSERT_EQ(A_TRANSPORT_VERSION_MAX, a_Transport_GetMessageVersion(&message));

    a_Transport_MessageConnect(&message, 1000U);
    a_Transport_SerializeMessage(&message, A_TRANSPORT_PEER_ID_MAX - 1U, A_TRANSPORT_SEQUENCE_NUMBER_MAX - 1U);
    a_Transport_DeserializeMessage(&message);
    ASSERT_EQ(1U, a_Transport_GetMessageVersion(&message));
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

TEST(Transport, GetMessageMtu)
{
    a_Transport_Message_t message;
    std::uint8_t buffer[AETHER_TRANSPORT_MTU];
    a_Transport_MessageInitialize(&message, buffer, sizeof(buffer));

    ASSERT_EQ(A_TRANSPORT_MTU_MAX, a_Transport_GetMessageMtu(nullptr));
    ASSERT_EQ(A_TRANSPORT_MTU_MAX, a_Transport_GetMessageMtu(&message));

    a_Transport_MessageConnect(&message, 1000U);
    a_Transport_SerializeMessage(&message, A_TRANSPORT_PEER_ID_MAX - 1U, A_TRANSPORT_SEQUENCE_NUMBER_MAX - 1U);
    a_Transport_DeserializeMessage(&message);
    ASSERT_EQ(AETHER_TRANSPORT_MTU, a_Transport_GetMessageMtu(&message));
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
    a_Transport_GetMessageMtu(&message);
    ASSERT_EQ(1000U, a_Transport_GetMessageLease(&message));

    a_Transport_MessageInitialize(&message, buffer, sizeof(buffer));
    a_Transport_MessageAccept(&message, 5000U);
    a_Transport_SerializeMessage(&message, A_TRANSPORT_PEER_ID_MAX - 1U, A_TRANSPORT_SEQUENCE_NUMBER_MAX - 1U);
    a_Transport_DeserializeMessage(&message);
    a_Transport_GetMessageMtu(&message);
    ASSERT_EQ(5000U, a_Transport_GetMessageLease(&message));
}

TEST(Transport, GetMessageKeySize)
{
    a_Transport_Message_t message;
    std::uint8_t buffer[AETHER_TRANSPORT_MTU];
    const char *const key = "/foo/bar";
    a_Transport_MessageInitialize(&message, buffer, sizeof(buffer));

    ASSERT_EQ(SIZE_MAX, a_Transport_GetMessageKeySize(nullptr));
    ASSERT_EQ(SIZE_MAX, a_Transport_GetMessageKeySize(&message));

    a_Transport_MessageSubscribe(&message, key);
    a_Transport_SerializeMessage(&message, A_TRANSPORT_PEER_ID_MAX - 1U, A_TRANSPORT_SEQUENCE_NUMBER_MAX - 1U);
    a_Transport_DeserializeMessage(&message);
    ASSERT_EQ(strlen(key) + 1U, a_Transport_GetMessageKeySize(&message));

    a_Transport_MessageUnsubscribe(&message, key);
    a_Transport_SerializeMessage(&message, A_TRANSPORT_PEER_ID_MAX - 1U, A_TRANSPORT_SEQUENCE_NUMBER_MAX - 1U);
    a_Transport_DeserializeMessage(&message);
    ASSERT_EQ(strlen(key) + 1U, a_Transport_GetMessageKeySize(&message));
}

TEST(Transport, GetMessageKey)
{
    a_Transport_Message_t message;
    std::uint8_t buffer[AETHER_TRANSPORT_MTU];
    const char *const key = "/foo/bar";
    a_Transport_MessageInitialize(&message, buffer, sizeof(buffer));

    ASSERT_EQ(nullptr, a_Transport_GetMessageKey(nullptr));
    ASSERT_EQ(nullptr, a_Transport_GetMessageKey(&message));

    a_Transport_MessageSubscribe(&message, key);
    buffer[9U] = 0xFFU;
    a_Transport_SerializeMessage(&message, A_TRANSPORT_PEER_ID_MAX - 1U, A_TRANSPORT_SEQUENCE_NUMBER_MAX - 1U);
    a_Transport_DeserializeMessage(&message);
    a_Transport_GetMessageKeySize(&message);
    ASSERT_EQ(nullptr, a_Transport_GetMessageKey(&message));

    a_Transport_MessageSubscribe(&message, key);
    a_Transport_SerializeMessage(&message, A_TRANSPORT_PEER_ID_MAX - 1U, A_TRANSPORT_SEQUENCE_NUMBER_MAX - 1U);
    a_Transport_DeserializeMessage(&message);
    a_Transport_GetMessageKeySize(&message);
    ASSERT_STREQ(key, a_Transport_GetMessageKey(&message));

    a_Transport_MessageUnsubscribe(&message, key);
    a_Transport_SerializeMessage(&message, A_TRANSPORT_PEER_ID_MAX - 1U, A_TRANSPORT_SEQUENCE_NUMBER_MAX - 1U);
    a_Transport_DeserializeMessage(&message);
    a_Transport_GetMessageKeySize(&message);
    ASSERT_STREQ(key, a_Transport_GetMessageKey(&message));
}

TEST(Transport, GetMessageKeyHash)
{
    a_Transport_Message_t message;
    std::uint8_t buffer[AETHER_TRANSPORT_MTU];
    const char *const key = "/foo/bar";
    std::uint8_t data[] = {0x00U, 0x01, 0x02, 0x03};
    a_Transport_MessageInitialize(&message, buffer, sizeof(buffer));

    ASSERT_EQ(A_HASH_MAX, a_Transport_GetMessageKeyHash(nullptr));
    ASSERT_EQ(A_HASH_MAX, a_Transport_GetMessageKeyHash(&message));

    a_Transport_MessagePublish(&message, key, data, sizeof(data));
    a_Transport_SerializeMessage(&message, A_TRANSPORT_PEER_ID_MAX - 1U, A_TRANSPORT_SEQUENCE_NUMBER_MAX - 1U);
    a_Transport_DeserializeMessage(&message);
    ASSERT_EQ(a_Hash_String(key, strlen(key)), a_Transport_GetMessageKeyHash(&message));
}

TEST(Transport, GetMessageDataSize)
{
    a_Transport_Message_t message;
    std::uint8_t buffer[AETHER_TRANSPORT_MTU];
    const char *const key = "/foo/bar";
    std::uint8_t data[] = {0x00U, 0x01, 0x02, 0x03};
    a_Transport_MessageInitialize(&message, buffer, sizeof(buffer));

    ASSERT_EQ(SIZE_MAX, a_Transport_GetMessageDataSize(nullptr));
    ASSERT_EQ(SIZE_MAX, a_Transport_GetMessageDataSize(&message));

    a_Transport_MessagePublish(&message, key, data, sizeof(data));
    a_Transport_SerializeMessage(&message, A_TRANSPORT_PEER_ID_MAX - 1U, A_TRANSPORT_SEQUENCE_NUMBER_MAX - 1U);
    a_Transport_DeserializeMessage(&message);
    a_Transport_GetMessageKeyHash(&message);
    ASSERT_EQ(sizeof(data), a_Transport_GetMessageDataSize(&message));
}

TEST(Transport, GetMessageData)
{
    a_Transport_Message_t message;
    std::uint8_t buffer[AETHER_TRANSPORT_MTU];
    const char *const key = "/foo/bar";
    std::uint8_t data[] = {0x00U, 0x01, 0x02, 0x03};
    a_Transport_MessageInitialize(&message, buffer, sizeof(buffer));

    ASSERT_EQ(nullptr, a_Transport_GetMessageData(nullptr));
    ASSERT_EQ(nullptr, a_Transport_GetMessageData(&message));

    a_Transport_MessagePublish(&message, key, data, sizeof(data));
    a_Transport_SerializeMessage(&message, A_TRANSPORT_PEER_ID_MAX - 1U, A_TRANSPORT_SEQUENCE_NUMBER_MAX - 1U);
    a_Transport_DeserializeMessage(&message);
    a_Transport_GetMessageKeyHash(&message);
    a_Transport_GetMessageDataSize(&message);
    ASSERT_THAT(SPAN_FROM_VALUE(a_Transport_GetMessageData(&message), sizeof(data)), testing::ElementsAreArray(data));
}
