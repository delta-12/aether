#include "transport.h"

#include <stdint.h>
#include <stddef.h>

#include "buffer.h"
#include "err.h"
#include "leb128.h"
#include "tick.h"

#define A_TRANSPORT_SERIALIZE_BUFFER_SIZE (LEB128_MAX_SIZE(uint64_t) + LEB128_MAX_SIZE(a_Transport_PeerId_t) + LEB128_MAX_SIZE(a_Transport_SequenceNumber_t))

a_Err_t a_Transport_MessageInitialize(a_Transport_Message_t *const message, uint8_t *const buffer, const size_t size)
{
    a_Err_t error = A_ERR_SIZE;

    if (NULL == message)
    {
        error = A_ERR_NULL;
    }
    else if (size >= AETHER_TRANSPORT_MTU)
    {
        error = a_Buffer_Initialize(&message->buffer, buffer, AETHER_TRANSPORT_MTU);
    }

    return error;
}

a_Err_t a_Transport_MessageConnect(a_Transport_Message_t *const message, const a_Tick_Ms_t lease)
{
    a_Err_t error = A_ERR_NULL;

    if (NULL != message)
    {
        message->header = A_TRANSPORT_HEADER_CONNECT;

        (void)a_Buffer_Clear(&message->buffer);

        /* TODO encode version and MTU */

        size_t size = Leb128_Encode64(lease, a_Buffer_GetWrite(&message->buffer), a_Buffer_GetWriteSize(&message->buffer));
        (void)a_Buffer_SetWrite(&message->buffer, size);

        error = A_ERR_NONE;
    }

    return error;
}

a_Err_t a_Transport_MessageAccept(a_Transport_Message_t *const message, const a_Tick_Ms_t lease)
{
    a_Err_t error = A_ERR_NULL;

    if (NULL != message)
    {
        message->header = A_TRANSPORT_HEADER_ACCEPT;

        (void)a_Buffer_Clear(&message->buffer);

        size_t size = Leb128_Encode64(lease, a_Buffer_GetWrite(&message->buffer), a_Buffer_GetWriteSize(&message->buffer));
        (void)a_Buffer_SetWrite(&message->buffer, size);

        error = A_ERR_NONE;
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

a_Err_t a_Transport_SerializeMessage(a_Transport_Message_t *const message, const a_Transport_PeerId_t peer_id, const a_Transport_SequenceNumber_t sequence_number)
{
    a_Err_t error = A_ERR_NULL;

    if (NULL != message)
    {
        a_Buffer_t serialize_buffer;
        uint8_t    serialize_data[A_TRANSPORT_SERIALIZE_BUFFER_SIZE];
        (void)a_Buffer_Initialize(&serialize_buffer, serialize_data, sizeof(serialize_data));

        size_t size = Leb128_Encode64(message->header, a_Buffer_GetWrite(&serialize_buffer), a_Buffer_GetWriteSize(&serialize_buffer));
        (void)a_Buffer_SetWrite(&serialize_buffer, size);

        message->peer_id = peer_id;
        size             = Leb128_Encode32(message->peer_id, a_Buffer_GetWrite(&serialize_buffer), a_Buffer_GetWriteSize(&serialize_buffer));
        (void)a_Buffer_SetWrite(&serialize_buffer, size);

        message->sequence_number = sequence_number;
        size                     = Leb128_Encode64(message->sequence_number, a_Buffer_GetWrite(&serialize_buffer), a_Buffer_GetWriteSize(&serialize_buffer));
        (void)a_Buffer_SetWrite(&serialize_buffer, size);

        (void)a_Buffer_AppendLeft(&message->buffer, &serialize_buffer);

        error = A_ERR_NONE;
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
        size_t size = Leb128_Decode64((uint64_t *)&message->header, a_Buffer_GetRead(&message->buffer), a_Buffer_GetReadSize(&message->buffer));

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
            error = A_ERR_NONE;
        }
    }

    return error;
}

a_Buffer_t *a_Transport_GetMessageBuffer(a_Transport_Message_t *const message)
{
    a_Buffer_t *buffer = NULL;

    if (NULL != message)
    {
        buffer = &message->buffer;
    }

    return buffer;
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
        case A_TRANSPORT_HEADER_CLOSE:
        case A_TRANSPORT_HEADER_RENEW:
        case A_TRANSPORT_HEADER_SUBSCRIBE:
        case A_TRANSPORT_HEADER_PUBLISH:
        default:
            break;
        }
    }

    return lease;
}
