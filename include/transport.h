#ifndef AETHER_TRANSPORT_H
#define AETHER_TRANSPORT_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include "buffer.h"
#include "hash.h"
#include "tick.h"

typedef uint8_t  a_Transport_Version_t;
typedef uint32_t a_Transport_PeerId_t;
typedef uint64_t a_Transport_SequenceNumber_t;
typedef uint16_t a_Transport_Mtu_t;

#define A_TRANSPORT_VERSION_MAX         (a_Transport_Version_t)(UINT8_MAX)
#define A_TRANSPORT_PEER_ID_MAX         (a_Transport_PeerId_t)(UINT32_MAX)
#define A_TRANSPORT_SEQUENCE_NUMBER_MAX (a_Transport_SequenceNumber_t)(UINT64_MAX)
#define A_TRANSPORT_MTU_MAX             (a_Transport_Mtu_t)(UINT16_MAX)

#ifndef AETHER_TRANSPORT_MTU
#define AETHER_TRANSPORT_MTU (a_Transport_Mtu_t)2048U /* TODO ensure this is less than A_TRANSPORT_MTU_MAX \
                                                         and is large enough to hold theoretical maximum transport message \
                                                         including LEB128-encoded version, header, PID, SEQ  */
#endif                                                /* AETHER_TRANSPORT_MTU */

typedef enum
{
    A_TRANSPORT_HEADER_CONNECT,
    A_TRANSPORT_HEADER_ACCEPT,
    A_TRANSPORT_HEADER_CLOSE,
    A_TRANSPORT_HEADER_RENEW,
    A_TRANSPORT_HEADER_PUBLISH,
    A_TRANSPORT_HEADER_SUBSCRIBE,
    A_TRANSPORT_HEADER_UNSUBSCRIBE,
    A_TRANSPORT_HEADER_MAX
} a_Transport_Header_t;

typedef struct
{
    a_Transport_Version_t version;
    a_Transport_Header_t header;
    a_Transport_PeerId_t peer_id;
    a_Transport_SequenceNumber_t sequence_number;
    a_Buffer_t buffer;
    bool serialized;
    bool deserialized;
} a_Transport_Message_t;

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

a_Err_t a_Transport_MessageInitialize(a_Transport_Message_t *const message, uint8_t *const buffer, const size_t size);
void a_Transport_MessageReset(a_Transport_Message_t *const message);
a_Err_t a_Transport_MessageConnect(a_Transport_Message_t *const message, const a_Tick_Ms_t lease);
a_Err_t a_Transport_MessageAccept(a_Transport_Message_t *const message, const a_Tick_Ms_t lease);
a_Err_t a_Transport_MessageClose(a_Transport_Message_t *const message);
a_Err_t a_Transport_MessageRenew(a_Transport_Message_t *const message);
a_Err_t a_Transport_MessagePublish(a_Transport_Message_t *const message, const char *const key, const uint8_t *const data, const size_t data_size);
a_Err_t a_Transport_MessageSubscribe(a_Transport_Message_t *const message, const char *const key);
a_Err_t a_Transport_MessageUnsubscribe(a_Transport_Message_t *const message, const char *const key);
a_Err_t a_Transport_SerializeMessage(a_Transport_Message_t *const message, const a_Transport_PeerId_t peer_id, const a_Transport_SequenceNumber_t sequence_number);
a_Err_t a_Transport_DeserializeMessage(a_Transport_Message_t *const message);
a_Err_t a_Transport_CopyMessage(const a_Transport_Message_t *const message, a_Transport_Message_t *const copy);
void a_Transport_CopyString(char *const copy, const char *const string, const size_t size);
bool a_Transport_IsMessageSerialized(const a_Transport_Message_t *const message);
bool a_Transport_IsMessageDeserialized(const a_Transport_Message_t *const message);
size_t a_Transport_GetStringSize(const char *const string);
a_Buffer_t *a_Transport_GetBuffer(a_Transport_Message_t *const message);
a_Transport_Mtu_t a_Transport_GetMtu(const a_Transport_Message_t *const message);
a_Transport_Version_t a_Transport_GetMessageVersion(const a_Transport_Message_t *const message);
a_Transport_Header_t a_Transport_GetMessageHeader(const a_Transport_Message_t *const message);
a_Transport_PeerId_t a_Transport_GetMessagePeerId(const a_Transport_Message_t *const message);
a_Transport_SequenceNumber_t a_Transport_GetMessageSequenceNumber(const a_Transport_Message_t *const message);
a_Transport_Mtu_t a_Transport_GetMessageMtu(a_Transport_Message_t *const message);
a_Tick_Ms_t a_Transport_GetMessageLease(a_Transport_Message_t *const message);
size_t a_Transport_GetMessageKeySize(a_Transport_Message_t *const message);
char *a_Transport_GetMessageKey(const a_Transport_Message_t *const message);
a_Hash_t a_Transport_GetMessageKeyHash(a_Transport_Message_t *const message);
size_t a_Transport_GetMessageDataSize(const a_Transport_Message_t *const message);
uint8_t *a_Transport_GetMessageData(const a_Transport_Message_t *const message);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AETHER_TRANSPORT_H */
