#include "transport.h"

#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "buffer.h"
#include "error.h"
#include "leb128.h"

#define A_TRANSPORT_SERIALIZE_BUFFER_SIZE (LEB128_MAX_SIZE(A_HEADER_SIZE) + LEB128_MAX_SIZE(a_PeerId_t) + LEB128_MAX_SIZE(a_SequenceNumber_t))

a_Error_t a_Transport_MessageInitialize(a_Transport_Message_t *const message,
                                        const a_PeerId_t peer_id,
                                        const a_SequenceNumber_t sequence_number,
                                        uint8_t *const buffer,
                                        const size_t size)
{
    a_Error_t error = A_ERROR_NONE;

    if (NULL == message)
    {
        error = A_ERROR_NULL;
    }
    else if (size < AETHER_TRANSPORT_MTU)
    {
        error = A_ERROR_SIZE;
    }
    else
    {
        message->peer_id         = peer_id;
        message->sequence_number = sequence_number;

        error = a_Buffer_Initialize(&message->buffer, buffer, AETHER_TRANSPORT_MTU);
    }

    return error;
}

a_Error_t a_Transport_MessageOpen(a_Transport_Message_t *const message, const a_SessionId_t session_id, const a_Milliseconds_t lease)
{
    a_Error_t error = A_ERROR_NULL;

    if (NULL != message)
    {
        message->header = A_HEADER_OPEN;

        (void)a_Buffer_Clear(&message->buffer);

        size_t size = Leb128_Encode32(session_id, a_Buffer_GetWrite(&message->buffer), a_Buffer_GetWriteSize(&message->buffer));
        (void)a_Buffer_SetWrite(&message->buffer, size);

        size = Leb128_Encode64(lease, a_Buffer_GetWrite(&message->buffer), a_Buffer_GetWriteSize(&message->buffer));
        (void)a_Buffer_SetWrite(&message->buffer, size);

        error = A_ERROR_NONE;
    }

    return error;
}

a_Error_t a_Transport_MessageAccept(a_Transport_Message_t *const message, const a_SessionId_t session_id, const a_Milliseconds_t lease)
{
    a_Error_t error = A_ERROR_NULL;

    if (NULL != message)
    {
        message->header = A_HEADER_ACCEPT;

        (void)a_Buffer_Clear(&message->buffer);

        size_t size = Leb128_Encode32(session_id, a_Buffer_GetWrite(&message->buffer), a_Buffer_GetWriteSize(&message->buffer));
        (void)a_Buffer_SetWrite(&message->buffer, size);

        size = Leb128_Encode64(lease, a_Buffer_GetWrite(&message->buffer), a_Buffer_GetWriteSize(&message->buffer));
        (void)a_Buffer_SetWrite(&message->buffer, size);

        error = A_ERROR_NONE;
    }

    return error;
}

a_Error_t a_Transport_MessageClose(a_Transport_Message_t *const message, const a_SessionId_t session_id)
{
    a_Error_t error = A_ERROR_NULL;

    if (NULL != message)
    {
        message->header = A_HEADER_CLOSE;

        (void)a_Buffer_Clear(&message->buffer);

        size_t size = Leb128_Encode32(session_id, a_Buffer_GetWrite(&message->buffer), a_Buffer_GetWriteSize(&message->buffer));
        (void)a_Buffer_SetWrite(&message->buffer, size);

        error = A_ERROR_NONE;
    }

    return error;
}

a_Error_t a_Transport_MessageRenew(a_Transport_Message_t *const message, const a_SessionId_t session_id)
{
    a_Error_t error = A_ERROR_NULL;

    if (NULL != message)
    {
        message->header = A_HEADER_RENEW;

        (void)a_Buffer_Clear(&message->buffer);

        size_t size = Leb128_Encode32(session_id, a_Buffer_GetWrite(&message->buffer), a_Buffer_GetWriteSize(&message->buffer));
        (void)a_Buffer_SetWrite(&message->buffer, size);

        error = A_ERROR_NONE;
    }

    return error;
}

a_Error_t a_Transport_SerializeMessage(a_Transport_Message_t *const message)
{
    a_Buffer_t buffer;
    uint8_t    data[A_TRANSPORT_SERIALIZE_BUFFER_SIZE];
    a_Error_t  error = a_Buffer_Initialize(&buffer, data, sizeof(data));

    if (NULL == message)
    {
        error = A_ERROR_NULL;
    }
    else if (A_ERROR_NONE == error)
    {
        size_t size = Leb128_Encode8(message->header, a_Buffer_GetWrite(&buffer), a_Buffer_GetWriteSize(&buffer));
        (void)a_Buffer_SetWrite(&buffer, size);

        size = Leb128_Encode32(message->peer_id, a_Buffer_GetWrite(&buffer), a_Buffer_GetWriteSize(&buffer));
        (void)a_Buffer_SetWrite(&buffer, size);

        size = Leb128_Encode64(message->sequence_number, a_Buffer_GetWrite(&buffer), a_Buffer_GetWriteSize(&buffer));
        (void)a_Buffer_SetWrite(&buffer, size);

        (void)a_Buffer_AppendLeft(&message->buffer, &buffer);
    }

    return error;
}
