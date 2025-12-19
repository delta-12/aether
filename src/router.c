#include "router.h"

#include "err.h"
#include "hashmap.h"
#include "socket.h"
#include "transport.h"

/* TODO internally track PID+SEQ and return error to drop message if necessary*/

#ifndef AETHER_ROUTER_MAX_SESSIONS
#define AETHER_ROUTER_MAX_SESSIONS 16U
#endif /* AETHER_ROUTER_MAX_SESSIONS */

#ifndef AETHER_ROUTER_MAX_PEERS
#define AETHER_ROUTER_MAX_PEERS 256U
#endif /* AETHER_ROUTER_MAX_PEERS */

static a_Transport_PeerId_t         a_Router_PeerId         = 0U;
static a_Transport_SequenceNumber_t a_Router_SequenceNumber = 0U;
static a_Hashmap_t                  a_Router_Sockets;
static uint8_t                      a_Router_SocketsData[(sizeof(a_Router_SessionId_t) + sizeof(a_Socket_t)) * AETHER_ROUTER_MAX_SESSIONS];
static a_Hashmap_t                  a_Router_Peers;
static uint8_t                      a_Router_PeersData[(sizeof(a_Transport_PeerId_t) + sizeof(a_Transport_SequenceNumber_t)) * AETHER_ROUTER_MAX_PEERS];

a_Err_t a_Router_Initialize(const a_Transport_PeerId_t id)
{
    if (A_TRANSPORT_PEER_ID_MAX == id)
    {
        a_Router_PeerId = 0U; /* TODO randomly generate */
    }
    else
    {
        a_Router_PeerId = id;
    }

    /* TODO log peer id */

    a_Err_t error = a_Hashmap_Initialize(&a_Router_Sockets, a_Router_SocketsData, sizeof(a_Router_SocketsData), sizeof(a_Router_SessionId_t), sizeof(a_Socket_t));

    if (A_ERR_NONE == error)
    {
        error = a_Hashmap_Initialize(&a_Router_Peers, a_Router_PeersData, sizeof(a_Router_PeersData), sizeof(a_Transport_PeerId_t), sizeof(a_Transport_SequenceNumber_t));
    }

    return error;
}

a_Err_t a_Router_SessionAdd(const a_Router_SessionId_t id, a_Socket_t *const socket)
{
    a_Err_t error = A_ERR_DUPLICATE;

    if (NULL == a_Hashmap_Get(&a_Router_Sockets, &id))
    {
        error = a_Hashmap_Insert(&a_Router_Sockets, &id, socket);
    }

    return error;
}

a_Err_t a_Router_SessionDelete(const a_Router_SessionId_t id)
{
    return a_Hashmap_Remove(&a_Router_Sockets, &id);
}

a_Err_t a_Router_SessionMessageGet(const a_Router_SessionId_t id, a_Transport_Message_t *const message)
{
    a_Buffer_t *buffer = a_Transport_GetMessageBuffer(message);
    a_Err_t     error  = a_Socket_Receive(a_Hashmap_Get(&a_Router_Sockets, &id), buffer);

    if ((A_ERR_NONE == error) && (a_Buffer_GetReadSize(buffer) > 0U))
    {
        error = a_Transport_DeserializeMessage(message);

        const a_Transport_PeerId_t                peer_id                 = a_Transport_GetMessagePeerId(message);
        const a_Transport_SequenceNumber_t        sequence_number         = a_Transport_GetMessageSequenceNumber(message);
        const a_Transport_SequenceNumber_t *const current_sequence_number = a_Hashmap_Get(&a_Router_Peers, &peer_id);

        if (A_ERR_NONE != error)
        {
            /* Error deserializing message */
        }
        else if ((A_TRANSPORT_PEER_ID_MAX == peer_id) || (A_TRANSPORT_SEQUENCE_NUMBER_MAX == sequence_number))
        {
            error = A_ERR_SERIALIZATION;
        }
        else if ((NULL != current_sequence_number) && (sequence_number <= *current_sequence_number))
        {
            error = A_ERR_SEQUENCE;
        }
        else
        {
            error = a_Hashmap_Insert(&a_Router_Peers, &peer_id, &sequence_number);
            /* TODO forward to other sessions if applicable */
        }
    }

    if (A_ERR_NONE != error)
    {
        /* TODO log error */
    }

    return error;
}

a_Err_t a_Router_SessionMessageSend(const a_Router_SessionId_t id, a_Transport_Message_t *const message)
{
    (void)a_Transport_SerializeMessage(message, a_Router_PeerId, a_Router_SequenceNumber++);
    a_Err_t error = a_Socket_Send(a_Hashmap_Get(&a_Router_Sockets, &id), a_Transport_GetMessageBuffer(message));

    if (A_ERR_NONE != error)
    {
        /* TODO log error */
    }

    return error;
}
