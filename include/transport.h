#ifndef AETHER_TRANSPORT_H
#define AETHER_TRANSPORT_H

#include <stdint.h>
#include <stddef.h>

#include "buffer.h"
#include "tick.h"

#ifndef AETHER_TRANSPORT_MTU
#define AETHER_TRANSPORT_MTU 2048U /* TODO ensure this is less than SIZE_MAX \
                                      and is large enough to hold theoretical maximum transport message \
                                      including LEB128-encoded header, PID, SEQ  */
#endif                             /* AETHER_MTU */

typedef uint32_t a_Transport_PeerId_t;
typedef uint64_t a_Transport_SequenceNumber_t;
typedef uint32_t a_Transport_SessionId_t;

typedef enum
{
    A_HEADER_CONNECT,
    A_HEADER_ACCEPT,
    A_HEADER_CLOSE,
    A_HEADER_RENEW,
    A_HEADER_SUBSCRIBE,
    A_HEADER_PUBLISH
} a_Transport_Header_t;

typedef struct
{
    a_Transport_Header_t header;
    a_Transport_PeerId_t peer_id;
    a_Transport_SequenceNumber_t sequence_number;
    a_Buffer_t buffer;
} a_Transport_Message_t;

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

a_Err_t a_Transport_MessageInitialize(a_Transport_Message_t *const message,
                                      const a_Transport_PeerId_t peer_id,
                                      const a_Transport_SequenceNumber_t sequence_number,
                                      uint8_t *const buffer,
                                      const size_t size);
a_Err_t a_Transport_MessageConnect(a_Transport_Message_t *const message, const a_Transport_SessionId_t session_id, const a_Tick_Ms_t lease);
a_Err_t a_Transport_MessageAccept(a_Transport_Message_t *const message, const a_Transport_SessionId_t session_id, const a_Tick_Ms_t lease);
a_Err_t a_Transport_MessageClose(a_Transport_Message_t *const message, const a_Transport_SessionId_t session_id);
a_Err_t a_Transport_MessageRenew(a_Transport_Message_t *const message, const a_Transport_SessionId_t session_id);
/* TODO publish, subscribe messages */
a_Err_t a_Transport_SerializeMessage(a_Transport_Message_t *const message);
a_Err_t a_Transport_DeserializeMessage(a_Transport_Message_t *const message);
a_Buffer_t *a_Transport_GetMessageBuffer(a_Transport_Message_t *const message);
/* TODO get session id, lease, MTU, etc. */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AETHER_TRANSPORT_H */
