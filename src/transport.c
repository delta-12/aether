#include "transport.h"

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "buffer.h"
#include "err.h"
#include "hash.h"
#include "leb128.h"
#include "tick.h"

#define A_TRANSPORT_SERIALIZE_BUFFER_SIZE (LEB128_MAX_SIZE(a_Transport_Version_t) + LEB128_MAX_SIZE(uint64_t) + LEB128_MAX_SIZE(a_Transport_PeerId_t) + \
                                           LEB128_MAX_SIZE(a_Transport_SequenceNumber_t))
#define A_TRANSPORT_BUFFER_SIZE_MIN       (A_TRANSPORT_SERIALIZE_BUFFER_SIZE + LEB128_MAX_SIZE(a_Transport_Mtu_t) + LEB128_MAX_SIZE(a_Tick_Ms_t))
#define A_TRANSPORT_STRING_SIZE_MAX(mtu)  ((size_t)mtu - (A_TRANSPORT_SERIALIZE_BUFFER_SIZE + LEB128_MAX_SIZE(uint64_t)))

a_Err_t a_Transport_MessageInitialize(a_Transport_Message_t *const message, uint8_t *const buffer, const size_t size)
{
    a_Err_t error = A_ERR_SIZE;

    if (NULL == message)
    {
        error = A_ERR_NULL;
    }
    else if (size < A_TRANSPORT_BUFFER_SIZE_MIN)
    {
        error = A_ERR_SIZE;
    }
    else
    {
        size_t mtu = size;

        if (mtu > AETHER_TRANSPORT_MTU)
        {
            mtu = AETHER_TRANSPORT_MTU;
        }

        error = a_Buffer_Initialize(&message->buffer, buffer, mtu);

        a_Transport_MessageReset(message);
    }

    return error;
}

void a_Transport_MessageReset(a_Transport_Message_t *const message)
{
    if (NULL != message)
    {
        message->version         = A_TRANSPORT_VERSION_MAX;
        message->header          = A_TRANSPORT_HEADER_MAX;
        message->peer_id         = A_TRANSPORT_PEER_ID_MAX;
        message->sequence_number = A_TRANSPORT_SEQUENCE_NUMBER_MAX;
        message->serialized      = false;
        message->deserialized    = false;

        (void)a_Buffer_Clear(&message->buffer);
    }
}

a_Err_t a_Transport_MessageConnect(a_Transport_Message_t *const message, const a_Tick_Ms_t lease)
{
    a_Err_t error = A_ERR_NULL;

    if (NULL != message)
    {
        message->header = A_TRANSPORT_HEADER_CONNECT;

        (void)a_Buffer_Clear(&message->buffer);

        const size_t buffer_size = a_Buffer_GetWriteSize(&message->buffer);
        size_t       size        = Leb128_Encode16((a_Transport_Mtu_t)buffer_size, a_Buffer_GetWrite(&message->buffer), buffer_size);
        (void)a_Buffer_SetWrite(&message->buffer, size);

        size = Leb128_Encode64(lease, a_Buffer_GetWrite(&message->buffer), a_Buffer_GetWriteSize(&message->buffer));
        (void)a_Buffer_SetWrite(&message->buffer, size);

        error = A_ERR_NONE;
    }

    return error;
}

a_Err_t a_Transport_MessageAccept(a_Transport_Message_t *const message, const a_Tick_Ms_t lease)
{
    a_Err_t error = a_Transport_MessageConnect(message, lease);

    if (A_ERR_NONE == error)
    {
        message->header = A_TRANSPORT_HEADER_ACCEPT;
    }

    return error;
}

a_Err_t a_Transport_MessageClose(a_Transport_Message_t *const message)
{
    a_Err_t error = A_ERR_NULL;

    if (NULL != message)
    {
        message->header = A_TRANSPORT_HEADER_CLOSE;

        (void)a_Buffer_Clear(&message->buffer);

        error = A_ERR_NONE;
    }

    return error;
}

a_Err_t a_Transport_MessageRenew(a_Transport_Message_t *const message)
{
    a_Err_t error = A_ERR_NULL;

    if (NULL != message)
    {
        message->header = A_TRANSPORT_HEADER_RENEW;

        (void)a_Buffer_Clear(&message->buffer);

        error = A_ERR_NONE;
    }

    return error;
}

a_Err_t a_Transport_MessagePublish(a_Transport_Message_t *const message, const char *const key, const uint8_t *const data, const size_t data_size)
{
    a_Err_t error = A_ERR_NONE;

    if ((NULL == message) || (NULL == key) || (NULL == data))
    {
        error = A_ERR_NULL;
    }
    else if (0U == data_size)
    {
        error = A_ERR_SIZE;
    }
    else
    {
        (void)a_Buffer_Clear(&message->buffer);

        size_t         size     = A_TRANSPORT_STRING_SIZE_MAX(a_Transport_GetMtu(message));
        const uint64_t key_hash = a_Hash_String(key, size);

        if (data_size > size)
        {
            error = A_ERR_SIZE;
        }
        else
        {
            message->header = A_TRANSPORT_HEADER_PUBLISH;

            size = Leb128_Encode64(key_hash, a_Buffer_GetWrite(&message->buffer), a_Buffer_GetWriteSize(&message->buffer));
            (void)a_Buffer_SetWrite(&message->buffer, size);

            memcpy(a_Buffer_GetWrite(&message->buffer), data, data_size);
            (void)a_Buffer_SetWrite(&message->buffer, data_size);
        }
    }

    return error;
}

a_Err_t a_Transport_MessageSubscribe(a_Transport_Message_t *const message, const char *const key)
{
    a_Err_t error = A_ERR_NULL;

    if ((NULL != message) && (NULL != key))
    {
        (void)a_Buffer_Clear(&message->buffer);

        const uint64_t key_size = a_Transport_GetStringSize(key);

        if (key_size > A_TRANSPORT_STRING_SIZE_MAX(a_Transport_GetMtu(message)))
        {
            error = A_ERR_SIZE;
        }
        else
        {
            message->header = A_TRANSPORT_HEADER_SUBSCRIBE;

            size_t size = Leb128_Encode64(key_size, a_Buffer_GetWrite(&message->buffer), a_Buffer_GetWriteSize(&message->buffer));
            (void)a_Buffer_SetWrite(&message->buffer, size);

            a_Transport_CopyString((char *)a_Buffer_GetWrite(&message->buffer), key, key_size);
            (void)a_Buffer_SetWrite(&message->buffer, key_size);

            error = A_ERR_NONE;
        }
    }

    return error;
}

a_Err_t a_Transport_SerializeMessage(a_Transport_Message_t *const message, const a_Transport_PeerId_t peer_id, const a_Transport_SequenceNumber_t sequence_number)
{
    a_Err_t error = A_ERR_NULL;

    if (NULL != message)
    {
        a_Buffer_t serialize_buffer;
        uint8_t    serialize_data[A_TRANSPORT_SERIALIZE_BUFFER_SIZE];
        (void)a_Buffer_Initialize(&serialize_buffer, serialize_data, sizeof(serialize_data));

        size_t size = Leb128_Encode8((a_Transport_Version_t)AETHER_GIT_VERSION_MAJOR, a_Buffer_GetWrite(&serialize_buffer), a_Buffer_GetWriteSize(&serialize_buffer));
        (void)a_Buffer_SetWrite(&serialize_buffer, size);

        size = Leb128_Encode64(message->header, a_Buffer_GetWrite(&serialize_buffer), a_Buffer_GetWriteSize(&serialize_buffer));
        (void)a_Buffer_SetWrite(&serialize_buffer, size);

        message->peer_id = peer_id;
        size             = Leb128_Encode32(message->peer_id, a_Buffer_GetWrite(&serialize_buffer), a_Buffer_GetWriteSize(&serialize_buffer));
        (void)a_Buffer_SetWrite(&serialize_buffer, size);

        message->sequence_number = sequence_number;
        size                     = Leb128_Encode64(message->sequence_number, a_Buffer_GetWrite(&serialize_buffer), a_Buffer_GetWriteSize(&serialize_buffer));
        (void)a_Buffer_SetWrite(&serialize_buffer, size);

        (void)a_Buffer_AppendLeft(&message->buffer, &serialize_buffer);

        message->serialized = true;
        error               = A_ERR_NONE;
    }

    return error;
}

a_Err_t a_Transport_DeserializeMessage(a_Transport_Message_t *const message)
{
    a_Err_t error = A_ERR_SERIALIZATION;

    if (NULL == message)
    {
        error = A_ERR_NULL;
    }
    else if (a_Buffer_GetReadSize(&message->buffer) > 0U)
    {
        size_t size = Leb128_Decode8(&message->version, a_Buffer_GetRead(&message->buffer), a_Buffer_GetReadSize(&message->buffer));

        if (SIZE_MAX != size)
        {
            (void)a_Buffer_SetRead(&message->buffer, size);
            uint64_t header;
            size            = Leb128_Decode64(&header, a_Buffer_GetRead(&message->buffer), a_Buffer_GetReadSize(&message->buffer));
            message->header = header;
        }

        if (SIZE_MAX != size)
        {
            (void)a_Buffer_SetRead(&message->buffer, size);
            size = Leb128_Decode32(&message->peer_id, a_Buffer_GetRead(&message->buffer), a_Buffer_GetReadSize(&message->buffer));
        }

        if (SIZE_MAX != size)
        {
            (void)a_Buffer_SetRead(&message->buffer, size);
            size = Leb128_Decode64(&message->sequence_number, a_Buffer_GetRead(&message->buffer), a_Buffer_GetReadSize(&message->buffer));
        }

        if (SIZE_MAX != size)
        {
            (void)a_Buffer_SetRead(&message->buffer, size);
            message->deserialized = true;
            error                 = A_ERR_NONE;
        }
    }

    return error;
}

a_Err_t a_Transport_CopyMessage(const a_Transport_Message_t *const message, a_Transport_Message_t *const copy)
{
    a_Err_t error = A_ERR_NULL;

    if ((NULL != message) && (NULL != copy))
    {
        copy->header          = message->header;
        copy->peer_id         = message->peer_id;
        copy->sequence_number = message->sequence_number;
        copy->serialized      = message->serialized;
        copy->deserialized    = message->deserialized;

        error = a_Buffer_Copy(&copy->buffer, &message->buffer);
    }

    return error;
}

void a_Transport_CopyString(char *const copy, const char *const string, const size_t size)
{
    if ((NULL != copy) && (NULL != string) && (size > 0U))
    {
        memcpy(copy, string, size);
        *(copy + (size - 1U)) = '\0';
    }
}

bool a_Transport_IsMessageSerialized(const a_Transport_Message_t *const message)
{
    bool serialized = false;

    if (NULL != message)
    {
        serialized = message->serialized;
    }

    return serialized;
}

bool a_Transport_IsMessageDeserialized(const a_Transport_Message_t *const message)
{
    bool deserialized = false;

    if (NULL != message)
    {
        deserialized = message->deserialized;
    }

    return deserialized;
}

size_t a_Transport_GetStringSize(const char *const string)
{
    size_t size = 0U;

    if (NULL != string)
    {
        size = strnlen(string, A_TRANSPORT_STRING_SIZE_MAX(AETHER_TRANSPORT_MTU) - 1U) + 1U;
    }

    return size;
}

a_Buffer_t *a_Transport_GetBuffer(a_Transport_Message_t *const message)
{
    a_Buffer_t *buffer = NULL;

    if (NULL != message)
    {
        buffer = &message->buffer;
    }

    return buffer;
}

a_Transport_Mtu_t a_Transport_GetMtu(const a_Transport_Message_t *const message)
{
    a_Transport_Mtu_t mtu = A_TRANSPORT_MTU_MAX;

    if (NULL != message)
    {
        mtu = (a_Transport_Mtu_t)a_Buffer_GetCapacity(&message->buffer);
    }

    return mtu;
}

a_Transport_Version_t a_Transport_GetMessageVersion(const a_Transport_Message_t *const message)
{
    a_Transport_Version_t version = A_TRANSPORT_VERSION_MAX;

    if (NULL != message)
    {
        version = message->version;
    }

    return version;
}

a_Transport_Header_t a_Transport_GetMessageHeader(const a_Transport_Message_t *const message)
{
    a_Transport_Header_t header = A_TRANSPORT_HEADER_MAX;

    if (NULL != message)
    {
        switch (message->header)
        {
        case A_TRANSPORT_HEADER_CONNECT:
        case A_TRANSPORT_HEADER_ACCEPT:
        case A_TRANSPORT_HEADER_CLOSE:
        case A_TRANSPORT_HEADER_RENEW:
        case A_TRANSPORT_HEADER_SUBSCRIBE:
        case A_TRANSPORT_HEADER_PUBLISH:
            header = message->header;
            break;
        default:
            break;
        }
    }

    return header;
}

a_Transport_PeerId_t a_Transport_GetMessagePeerId(const a_Transport_Message_t *const message)
{
    a_Transport_PeerId_t peer_id = A_TRANSPORT_PEER_ID_MAX;

    if (NULL != message)
    {
        peer_id = message->peer_id;
    }

    return peer_id;
}

a_Transport_SequenceNumber_t a_Transport_GetMessageSequenceNumber(const a_Transport_Message_t *const message)
{
    a_Transport_SequenceNumber_t sequence_number = A_TRANSPORT_SEQUENCE_NUMBER_MAX;

    if (NULL != message)
    {
        sequence_number = message->sequence_number;
    }

    return sequence_number;
}

a_Transport_Mtu_t a_Transport_GetMessageMtu(a_Transport_Message_t *const message)
{
    a_Transport_Mtu_t mtu = A_TRANSPORT_MTU_MAX;

    if (NULL != message)
    {
        size_t size;
        switch (message->header)
        {
        case A_TRANSPORT_HEADER_CONNECT:
        case A_TRANSPORT_HEADER_ACCEPT:
            size = Leb128_Decode16(&mtu, a_Buffer_GetRead(&message->buffer), a_Buffer_GetReadSize(&message->buffer));
            if (SIZE_MAX != size)
            {
                (void)a_Buffer_SetRead(&message->buffer, size);
            }
            else
            {
                mtu = A_TRANSPORT_MTU_MAX;
            }
            break;
        default:
            break;
        }
    }

    return mtu;
}

a_Tick_Ms_t a_Transport_GetMessageLease(a_Transport_Message_t *const message)
{
    a_Tick_Ms_t lease = A_TICK_MS_MAX;

    if (NULL != message)
    {
        size_t size;
        switch (message->header)
        {
        case A_TRANSPORT_HEADER_CONNECT:
        case A_TRANSPORT_HEADER_ACCEPT:
            size = Leb128_Decode64(&lease, a_Buffer_GetRead(&message->buffer), a_Buffer_GetReadSize(&message->buffer));
            if (SIZE_MAX != size)
            {
                (void)a_Buffer_SetRead(&message->buffer, size);
            }
            else
            {
                lease = A_TICK_MS_MAX;
            }
            break;
        default:
            break;
        }
    }

    return lease;
}

size_t a_Transport_GetMessageKeySize(a_Transport_Message_t *const message)
{
    size_t key_size = SIZE_MAX;

    if ((NULL != message) && (A_TRANSPORT_HEADER_SUBSCRIBE == message->header))
    {
        uint64_t     key_size_u64;
        const size_t size = Leb128_Decode64(&key_size_u64, a_Buffer_GetRead(&message->buffer), a_Buffer_GetReadSize(&message->buffer));

        if (SIZE_MAX != size)
        {
            key_size = key_size_u64;
            (void)a_Buffer_SetRead(&message->buffer, size);
        }
    }

    return key_size;
}

char *a_Transport_GetMessageKey(const a_Transport_Message_t *const message)
{
    char *key = NULL;

    if ((NULL != message) && (A_TRANSPORT_HEADER_SUBSCRIBE == message->header))
    {
        /* TODO return NULL if string is not valid, i.e. not null terminated */
        key = (char *)a_Buffer_GetRead(&message->buffer);
    }

    return key;
}

a_Hash_t a_Transport_GetMessageKeyHash(a_Transport_Message_t *const message)
{
    a_Hash_t key_hash = A_HASH_MAX;

    if ((NULL != message) && (A_TRANSPORT_HEADER_PUBLISH == message->header))
    {
        const size_t size = Leb128_Decode64(&key_hash, a_Buffer_GetRead(&message->buffer), a_Buffer_GetReadSize(&message->buffer));

        if (SIZE_MAX != size)
        {
            (void)a_Buffer_SetRead(&message->buffer, size);
        }
        else
        {
            key_hash = A_HASH_MAX;
        }
    }

    return key_hash;
}

size_t a_Transport_GetMessageDataSize(const a_Transport_Message_t *const message)
{
    size_t data_size = SIZE_MAX;

    if ((NULL != message) && (A_TRANSPORT_HEADER_PUBLISH == message->header))
    {
        data_size = a_Buffer_GetReadSize(&message->buffer);
    }

    return data_size;
}

uint8_t *a_Transport_GetMessageData(const a_Transport_Message_t *const message)
{
    uint8_t *data = NULL;

    if ((NULL != message) && (A_TRANSPORT_HEADER_PUBLISH == message->header))
    {
        data = a_Buffer_GetRead(&message->buffer);
    }

    return data;
}
