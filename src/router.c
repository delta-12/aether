#include "router.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "err.h"
#include "hash.h"
#include "hashmap.h"
#include "log.h"
#include "memory.h"
#include "random.h"
#include "socket.h"
#include "tick.h"
#include "transport.h"

#ifndef AETHER_SESSION_RETRIES
#define AETHER_SESSION_RETRIES 5U
#endif /* AETHER_SESSION_RETRIES */

#ifndef AETHER_SESSION_LEASE
#define AETHER_SESSION_LEASE (a_Tick_Ms_t)500U
#endif /* AETHER_SESSION_LEASE */

typedef struct a_Router_Deletion          a_Router_Deletion_t;
typedef struct a_Router_SubscriberSession a_Router_SubscriberSession_t;

typedef enum
{
    A_ROUTER_SESSION_STATE_CONNECT,
    A_ROUTER_SESSION_STATE_ACCEPT,
    A_ROUTER_SESSION_STATE_OPEN,
    A_ROUTER_SESSION_STATE_CLOSED,
    A_ROUTER_SESSION_STATE_FAILED
} a_Router_SessionState_t;

struct a_Router_Deletion
{
    a_Router_SessionId_t id;
    a_Router_Deletion_t *next;
};

typedef struct
{
    a_Socket_t socket;
    a_Router_SessionState_t state;
    size_t retries;
    a_Tick_Ms_t lease;
    a_Tick_Ms_t last_renew_received;
    a_Tick_Ms_t last_renew_sent;
    a_Transport_Message_t message;
    a_Router_Deletion_t deletion;
    bool retain;
    bool accept_sent;
} a_Router_Session_t;

struct a_Router_SubscriberSession
{
    a_Router_SessionId_t id;
    a_Router_SubscriberSession_t *next;
};

typedef struct
{
    a_Router_SubscriberSession_t *sessions;
    void (*function)(const char *const key, const uint8_t *const data, const size_t size, void *arg);
    void *arg;
    char *key;
} a_Router_Subscription_t;

static const char *const            a_Router_LogTag                     = "ROUTER";
static a_Transport_PeerId_t         a_Router_PeerId                     = 0U;
static a_Transport_SequenceNumber_t a_Router_SequenceNumber             = 0U;
static bool                         a_Router_RoutingEnabled             = true;
static a_Router_Deletion_t *        a_Router_DeleteList                 = NULL;
static bool                         a_Router_SessionsInitialized        = false;
static bool                         a_Router_SequenceNumbersInitialized = false;
static bool                         a_Router_SubscriptionsInitialized   = false;
static a_Hashmap_t                  a_Router_Sessions;
static a_Hashmap_t                  a_Router_SequenceNumbers;
static a_Hashmap_t                  a_Router_Subscriptions;

static void a_Router_SerializeMessage(a_Transport_Message_t *const message);
static a_Err_t a_Router_SessionMessageSend(const a_Router_SessionId_t id, a_Router_Session_t *const session);
static a_Err_t a_Router_SessionMessageReceive(const a_Router_SessionId_t id, a_Router_Session_t *const session);
static a_Err_t a_Router_SessionConnect(const a_Router_SessionId_t id, a_Router_Session_t *const session);
static a_Err_t a_Router_SessionAccept(const a_Router_SessionId_t id, a_Router_Session_t *const session);
static a_Err_t a_Router_SessionOpen(const a_Router_SessionId_t id, a_Router_Session_t *const session);
static a_Err_t a_Router_SessionClose(const a_Router_SessionId_t id, a_Router_Session_t *const session);
static a_Err_t a_Router_SessionHandleConnectAndAccept(const a_Router_SessionId_t id, a_Router_Session_t *const session);
static a_Err_t a_Router_SessionHandlePublish(const a_Router_SessionId_t id, a_Router_Session_t *const session);
static a_Err_t a_Router_SessionHandleSubscribe(const a_Router_SessionId_t id, a_Router_Session_t *const session);
static a_Err_t a_Router_SessionHandleUnsubscribe(const a_Router_SessionId_t id, a_Router_Session_t *const session);
static a_Err_t a_Router_RemoveSubscriberSession(a_Router_Subscription_t *const subscription, const a_Hash_t hash, const a_Router_SessionId_t id);
static void a_Router_SessionTaskCallback(const void *const key, const size_t key_size, void *const value, const size_t value_size, const void *const arg);
static void a_Router_SessionSubscribeCallback(const void *const key, const size_t key_size, void *const value, const size_t value_size, const void *const arg);
static void a_Router_SessionUnsubscribeCallback(const void *const key, const size_t key_size, void *const value, const size_t value_size, const void *const arg);
static void a_Router_RemoveSubscriberSessionCallback(const void *const key, const size_t key_size, void *const value, const size_t value_size, const void *const arg);
static void a_Router_FreeSubscriptionCallback(const void *const key, const size_t key_size, void *const value, const size_t value_size, const void *const arg);
static void a_Router_SessionForwardSubscribeCallback(const void *const key, const size_t key_size, void *const value, const size_t value_size, const void *const arg);
static void a_Router_SessionSendSubscriptionsCallback(const void *const key, const size_t key_size, void *const value, const size_t value_size, const void *const arg);

a_Err_t a_Router_Initialize(const a_Transport_PeerId_t id)
{
    if (A_TRANSPORT_PEER_ID_MAX == id)
    {
        a_Router_PeerId = a_Random_Get32();
    }
    else
    {
        a_Router_PeerId = id;
    }

    A_LOG_DEBUG(a_Router_LogTag, "Peer ID set to %#x", a_Router_PeerId);

    a_Err_t error = a_Hashmap_Initialize(&a_Router_Sessions);

    if (A_ERR_NONE == error)
    {
        a_Router_SessionsInitialized = true;
        error                        = a_Hashmap_Initialize(&a_Router_SequenceNumbers);
    }

    if (A_ERR_NONE == error)
    {
        a_Router_SequenceNumbersInitialized = true;
        error                               = a_Hashmap_Initialize(&a_Router_Subscriptions);
    }

    if (A_ERR_NONE == error)
    {
        a_Router_SubscriptionsInitialized = true;
    }

    return error;
}

void a_Router_Deinitialize(void)
{
    if (a_Router_SessionsInitialized)
    {
        a_Hashmap_Deinitialize(&a_Router_Sessions);
        a_Router_SessionsInitialized = false;
    }

    if (a_Router_SequenceNumbersInitialized)
    {
        a_Hashmap_Deinitialize(&a_Router_SequenceNumbers);
        a_Router_SequenceNumbersInitialized = false;
    }

    if (a_Router_SubscriptionsInitialized)
    {
        a_Hashmap_ForEach(&a_Router_Subscriptions, a_Router_FreeSubscriptionCallback, NULL);
        a_Hashmap_Deinitialize(&a_Router_Subscriptions);
        a_Router_SubscriptionsInitialized = false;
    }
}

void a_Routing_EnableRouting(const bool enable)
{
    a_Router_RoutingEnabled = enable;

    if (a_Router_RoutingEnabled)
    {
        A_LOG_DEBUG(a_Router_LogTag, "Routing enabled");
    }
    else
    {
        A_LOG_DEBUG(a_Router_LogTag, "Routing disabled");
    }
}

void a_Router_Task(void)
{
    a_Hashmap_ForEach(&a_Router_Sessions, a_Router_SessionTaskCallback, NULL);

    while (NULL != a_Router_DeleteList)
    {
        a_Router_Deletion_t *const next = a_Router_DeleteList->next;

        /* TODO handle deletion failure (otherwise session will remain in closed state and SessionClose will continuously be called on it) */
        (void)a_Router_SessionDelete(a_Router_DeleteList->id);

        a_Router_DeleteList = next;
    }
}

a_Err_t a_Router_SessionAdd(const a_Router_SessionId_t id, const a_Socket_t *const socket, uint8_t *const buffer, const size_t size, const bool retain)
{
    a_Err_t error = A_ERR_DUPLICATE;

    if (NULL == socket)
    {
        error = A_ERR_NULL;
    }
    else if (NULL == a_Hashmap_Get(&a_Router_Sessions, &id, sizeof(a_Router_SessionId_t)))
    {
        a_Router_Session_t session;
        error = a_Hashmap_Insert(&a_Router_Sessions, &id, sizeof(a_Router_SessionId_t), &session, sizeof(a_Router_Session_t));

        if (A_ERR_NONE == error)
        {
            a_Router_Session_t *new_session = (a_Router_Session_t *)a_Hashmap_Get(&a_Router_Sessions, &id, sizeof(a_Router_SessionId_t));

            new_session->socket = *socket;
            new_session->state  = A_ROUTER_SESSION_STATE_CONNECT;
            new_session->retain = retain;
            error               = a_Transport_MessageInitialize(&new_session->message, buffer, size);

            A_LOG_DEBUG(a_Router_LogTag, "Session %#x added", id);
        }
    }

    return error;
}

a_Err_t a_Router_SessionDelete(const a_Router_SessionId_t id)
{
    a_Err_t                   error   = A_ERR_NONE;
    a_Router_Session_t *const session = a_Hashmap_Get(&a_Router_Sessions, &id, sizeof(a_Router_SessionId_t));

    if (NULL != session)
    {
        if ((A_ROUTER_SESSION_STATE_CLOSED != session->state) && (A_ROUTER_SESSION_STATE_FAILED != session->state))
        {
            session->state = A_ROUTER_SESSION_STATE_CLOSED;

            a_Transport_MessageReset(&session->message);
            (void)a_Transport_MessageClose(&session->message);
            error = a_Router_SessionMessageSend(id, session);

            if (A_ERR_NONE == error)
            {
                error = a_Socket_Stop(&session->socket);
            }
        }

        if (A_ERR_NONE == error)
        {
            a_Hashmap_ForEach(&a_Router_Subscriptions, a_Router_RemoveSubscriberSessionCallback, &id);

            error = a_Hashmap_Remove(&a_Router_Sessions, &id, sizeof(a_Router_SessionId_t));
        }

        if (A_ERR_NONE == error)
        {
            A_LOG_DEBUG(a_Router_LogTag, "Session %#x deleted", id);
        }
        else
        {
            A_LOG_ERROR(a_Router_LogTag, "Session %#x failed to delete with error %s", id, a_Err_ToString(error));
        }
    }

    return error;
}

a_Err_t a_Router_Declare(const char *const key)
{
    a_Err_t error = A_ERR_NONE;

    if (NULL == key)
    {
        error = A_ERR_NULL;
    }
    else
    {
        const size_t                         key_size     = a_Transport_GetStringSize(key);
        const a_Hash_t                       hash         = a_Hash_String(key, key_size);
        const a_Router_Subscription_t *const subscription = a_Hashmap_Get(&a_Router_Subscriptions, &hash, sizeof(hash));

        if (NULL == subscription)
        {
            a_Router_Subscription_t new_subscription = {
                .sessions = NULL,
                .function = NULL,
                .arg      = NULL,
                .key      = a_malloc(key_size),
            };
            a_Transport_CopyString(new_subscription.key, key, key_size);

            error = a_Hashmap_Insert(&a_Router_Subscriptions, &hash, sizeof(hash), &new_subscription, sizeof(new_subscription));
        }
    }

    if (A_ERR_NONE != error)
    {
        A_LOG_ERROR(a_Router_LogTag, "Failed to declare key with error %s", a_Err_ToString(error));
    }

    return error;
}

a_Err_t a_Router_Publish(const char *const key, const uint8_t *const data, const size_t size)
{
    a_Err_t error = A_ERR_NONE;

    if ((NULL == key) || (NULL == data))
    {
        error = A_ERR_NULL;
    }
    else if (0U == size)
    {
        error = A_ERR_SIZE;
    }
    else
    {
        const a_Hash_t                       hash         = a_Hash_String(key, a_Transport_GetStringSize(key));
        const a_Router_Subscription_t *const subscription = a_Hashmap_Get(&a_Router_Subscriptions, &hash, sizeof(hash));

        if (NULL != subscription)
        {
            a_Router_SubscriberSession_t *subscriber_session = subscription->sessions;

            while (NULL != subscriber_session)
            {
                a_Router_Session_t *session = a_Hashmap_Get(&a_Router_Sessions, &subscriber_session->id, sizeof(subscriber_session->id));

                a_Transport_MessageReset(&session->message);
                a_Err_t send_error = a_Transport_MessagePublish(&session->message, key, data, size);

                if (A_ERR_NONE == send_error)
                {
                    (void)a_Transport_SerializeMessage(&session->message, a_Router_PeerId, a_Router_SequenceNumber);

                    send_error = a_Socket_Send(&session->socket, a_Transport_GetBuffer(&session->message));
                }

                if (A_ERR_NONE != send_error)
                {
                    A_LOG_ERROR(a_Router_LogTag, "Session %#x sending publish message with error %s", subscriber_session->id, a_Err_ToString(send_error));
                    error = send_error;
                }

                subscriber_session = subscriber_session->next;
            }

            a_Router_SequenceNumber++;
        }
    }

    return error;
}

a_Err_t a_Router_Subscribe(const char *const key, void (*callback)(const char *const key, const uint8_t *const data, const size_t size, void *arg), void *arg)
{
    a_Err_t error = A_ERR_NONE;

    if ((NULL == key) || (NULL == callback))
    {
        error = A_ERR_NULL;
    }
    else
    {
        const size_t             key_size     = a_Transport_GetStringSize(key);
        const a_Hash_t           hash         = a_Hash_String(key, key_size);
        a_Router_Subscription_t *subscription = a_Hashmap_Get(&a_Router_Subscriptions, &hash, sizeof(hash));

        if (NULL == subscription)
        {
            a_Router_Subscription_t new_subscription = {
                .sessions = NULL,
                .function = callback,
                .arg      = arg,
                .key      = a_malloc(key_size),
            };
            a_Transport_CopyString(new_subscription.key, key, key_size);

            error = a_Hashmap_Insert(&a_Router_Subscriptions, &hash, sizeof(hash), &new_subscription, sizeof(new_subscription));
        }
        else
        {
            subscription->function = callback;
            subscription->arg      = arg;
        }

        if (A_ERR_NONE == error)
        {
            a_Hashmap_ForEach(&a_Router_Sessions, a_Router_SessionSubscribeCallback, key);
            a_Router_SequenceNumber++;
        }
        else
        {
            A_LOG_ERROR(a_Router_LogTag, "Failed to register subscription with error %s", a_Err_ToString(error));
        }
    }

    return error;
}

a_Err_t a_Router_Unsubscribe(const char *const key)
{
    a_Err_t error = A_ERR_NONE;

    if (NULL == key)
    {
        error = A_ERR_NULL;
    }
    else
    {
        const a_Hash_t           hash         = a_Hash_String(key, a_Transport_GetStringSize(key));
        a_Router_Subscription_t *subscription = a_Hashmap_Get(&a_Router_Subscriptions, &hash, sizeof(hash));

        if (NULL != subscription)
        {
            subscription->function = NULL;
            subscription->arg      = NULL;

            if (NULL == subscription->sessions)
            {
                a_Hashmap_ForEach(&a_Router_Sessions, a_Router_SessionUnsubscribeCallback, key);
                a_Router_SequenceNumber++;

                a_free(subscription->key);
                error = a_Hashmap_Remove(&a_Router_Subscriptions, &hash, sizeof(hash));
            }
        }
    }

    return error;
}

static void a_Router_SerializeMessage(a_Transport_Message_t *const message)
{
    (void)a_Transport_SerializeMessage(message, a_Router_PeerId, a_Router_SequenceNumber);
    a_Router_SequenceNumber++;
}

static a_Err_t a_Router_SessionMessageSend(const a_Router_SessionId_t id, a_Router_Session_t *const session)
{
    a_Router_SerializeMessage(&session->message);

    a_Err_t error = a_Socket_Send(&session->socket, a_Transport_GetBuffer(&session->message));

    if (A_ERR_NONE != error)
    {
        A_LOG_ERROR(a_Router_LogTag, "Session %#x failed to send message with error %s", id, a_Err_ToString(error));
    }

    return error;
}

static a_Err_t a_Router_SessionMessageReceive(const a_Router_SessionId_t id, a_Router_Session_t *const session)
{
    a_Transport_MessageReset(&session->message);

    a_Buffer_t *const buffer = a_Transport_GetBuffer(&session->message);
    a_Err_t           error  = a_Socket_Receive(&session->socket, buffer);

    if ((A_ERR_NONE == error) && (a_Buffer_GetReadSize(buffer) > 0U))
    {
        error = a_Transport_DeserializeMessage(&session->message);

        const a_Transport_PeerId_t                peer_id                 = a_Transport_GetMessagePeerId(&session->message);
        const a_Transport_SequenceNumber_t        sequence_number         = a_Transport_GetMessageSequenceNumber(&session->message);
        const a_Transport_SequenceNumber_t *const current_sequence_number = a_Hashmap_Get(&a_Router_SequenceNumbers, &peer_id, sizeof(a_Transport_PeerId_t));

        if (A_ERR_NONE != error)
        {
            /* Error deserializing message */
        }
        else if ((A_TRANSPORT_PEER_ID_MAX == peer_id) || (A_TRANSPORT_SEQUENCE_NUMBER_MAX == sequence_number))
        {
            error = A_ERR_SERIALIZATION;
        }
        else if ((NULL == current_sequence_number) || (sequence_number > *current_sequence_number))
        {
            error = a_Hashmap_Insert(&a_Router_SequenceNumbers, &peer_id, sizeof(a_Transport_PeerId_t), &sequence_number, sizeof(a_Transport_SequenceNumber_t));
        }
    }

    if (A_ERR_NONE != error)
    {
        A_LOG_ERROR(a_Router_LogTag, "Session %#x failed to receive message with error %s", id, a_Err_ToString(error));
    }

    return error;
}

static a_Err_t a_Router_SessionConnect(const a_Router_SessionId_t id, a_Router_Session_t *const session)
{
    a_Err_t error = a_Socket_Start(&session->socket);

    if (A_ERR_NONE == error)
    {
        session->lease = AETHER_SESSION_LEASE;

        a_Transport_MessageReset(&session->message);
        (void)a_Transport_MessageConnect(&session->message, session->lease);

        error = a_Router_SessionMessageSend(id, session);
    }

    if (A_ERR_NONE == error)
    {
        session->state               = A_ROUTER_SESSION_STATE_ACCEPT;
        session->retries             = 1U;
        session->last_renew_received = a_Tick_GetTick();
        session->accept_sent         = false;

        A_LOG_DEBUG(a_Router_LogTag, "Session %#x connecting", id);
    }
    else
    {
        session->state = A_ROUTER_SESSION_STATE_FAILED;

        A_LOG_ERROR(a_Router_LogTag, "Session %#x failed to connect with error %s", id, a_Err_ToString(error));
    }

    return error;
}

static a_Err_t a_Router_SessionAccept(const a_Router_SessionId_t id, a_Router_Session_t *const session)
{
    a_Err_t error = A_ERR_NONE;

    if (session->retries <= AETHER_SESSION_RETRIES)
    {
        const a_Tick_Ms_t tick = a_Tick_GetTick();

        if (a_Tick_GetElapsed(session->last_renew_received) > (session->lease * session->retries))
        {
            session->retries++;
            session->last_renew_received = tick;
        }

        error = a_Router_SessionMessageReceive(id, session);

        if ((A_ERR_NONE != error) || !a_Transport_IsMessageDeserialized(&session->message))
        {
            /* Error receiving message or no message received */
        }
        else if (A_TRANSPORT_HEADER_CONNECT == a_Transport_GetMessageHeader(&session->message))
        {
            error                        = a_Router_SessionHandleConnectAndAccept(id, session);
            session->last_renew_received = tick;
        }
        else if (A_TRANSPORT_HEADER_ACCEPT == a_Transport_GetMessageHeader(&session->message))
        {
            error                        = a_Router_SessionHandleConnectAndAccept(id, session);
            session->last_renew_received = tick;

            if (session->accept_sent)
            {
                session->last_renew_sent = tick;
                session->state           = A_ROUTER_SESSION_STATE_OPEN;

                a_Hashmap_ForEach(&a_Router_Subscriptions, a_Router_SessionSendSubscriptionsCallback, &id);

                A_LOG_INFO(a_Router_LogTag, "Session %#x opened", id);
            }
        }
        else
        {
            error = A_ERR_SEQUENCE;
        }
    }
    else
    {
        session->state = A_ROUTER_SESSION_STATE_CLOSED;

        A_LOG_WARNING(a_Router_LogTag, "Session %#x not opened", id);
    }

    if (A_ERR_NONE != error)
    {
        session->state = A_ROUTER_SESSION_STATE_FAILED;

        A_LOG_ERROR(a_Router_LogTag, "Session %#x failed to open with error %s", id, a_Err_ToString(error));
    }

    return error;
}

static a_Err_t a_Router_SessionOpen(const a_Router_SessionId_t id, a_Router_Session_t *const session)
{
    a_Err_t           error = a_Router_SessionMessageReceive(id, session);
    const a_Tick_Ms_t tick  = a_Tick_GetTick();

    if ((A_ERR_NONE == error) && a_Transport_IsMessageDeserialized(&session->message))
    {
        switch (a_Transport_GetMessageHeader(&session->message))
        {
        case A_TRANSPORT_HEADER_ACCEPT:
            break;
        case A_TRANSPORT_HEADER_CLOSE:
            session->state = A_ROUTER_SESSION_STATE_CLOSED;
            break;
        case A_TRANSPORT_HEADER_RENEW:
            session->last_renew_received = tick;
            break;
        case A_TRANSPORT_HEADER_PUBLISH:
            session->last_renew_received = tick;
            error                        = a_Router_SessionHandlePublish(id, session);
            break;
        case A_TRANSPORT_HEADER_SUBSCRIBE:
            session->last_renew_received = tick;
            error                        = a_Router_SessionHandleSubscribe(id, session);
            break;
        case A_TRANSPORT_HEADER_UNSUBSCRIBE:
            session->last_renew_received = tick;
            error                        = a_Router_SessionHandleUnsubscribe(id, session);
            break;
        case A_TRANSPORT_HEADER_CONNECT:
        default:
            A_LOG_WARNING(a_Router_LogTag, "Session %#x received invalid header %d", id, a_Transport_GetMessageHeader(&session->message));
            break;
        }
    }

    if (a_Tick_GetElapsed(session->last_renew_received) > session->lease)
    {
        error = A_ERR_TIMEOUT;

        A_LOG_ERROR(a_Router_LogTag, "Session %#x timed out", id);
    }
    else if (a_Tick_GetElapsed(session->last_renew_sent) > (session->lease / 2U))
    {
        a_Transport_MessageReset(&session->message);
        (void)a_Transport_MessageRenew(&session->message);
        error                    = a_Router_SessionMessageSend(id, session);
        session->last_renew_sent = tick;
    }

    if (A_ERR_NONE != error)
    {
        session->state = A_ROUTER_SESSION_STATE_FAILED;
    }

    return error;
}

static a_Err_t a_Router_SessionClose(const a_Router_SessionId_t id, a_Router_Session_t *const session)
{
    a_Err_t error = a_Socket_Stop(&session->socket);

    if (A_ERR_NONE != error)
    {
        /* Failed to stop socket */
    }
    else if (session->retain)
    {
        a_Hashmap_ForEach(&a_Router_Subscriptions, a_Router_RemoveSubscriberSessionCallback, &id);
        session->state = A_ROUTER_SESSION_STATE_CONNECT;
    }
    else
    {
        session->deletion.id   = id;
        session->deletion.next = a_Router_DeleteList;
        a_Router_DeleteList    = &session->deletion;
    }

    if (A_ERR_NONE == error)
    {
        A_LOG_INFO(a_Router_LogTag, "Session %#x closed", id);
    }
    else
    {
        A_LOG_ERROR(a_Router_LogTag, "Failed to close session %#x with error %s", id, a_Err_ToString(error));
    }

    return error;
}

static a_Err_t a_Router_SessionHandleConnectAndAccept(const a_Router_SessionId_t id, a_Router_Session_t *const session)
{
    /* TODO handle version mismatch */

    a_Err_t                 error = A_ERR_NONE;
    const a_Transport_Mtu_t mtu   = a_Transport_GetMessageMtu(&session->message);
    const a_Tick_Ms_t       lease = a_Transport_GetMessageLease(&session->message);

    if (A_TRANSPORT_MTU_MAX == mtu)
    {
        error = A_ERR_SERIALIZATION;

        A_LOG_WARNING(a_Router_LogTag, "Session %#x received invalid MTU", id);
    }
    else if (mtu < a_Transport_GetMtu(&session->message))
    {
        a_Buffer_t *const buffer = a_Transport_GetBuffer(&session->message);

        (void)a_Buffer_Clear(buffer);

        error = a_Transport_MessageInitialize(&session->message, a_Buffer_GetWrite(buffer), mtu);
    }

    if (A_TICK_MS_MAX == lease)
    {
        error = A_ERR_SERIALIZATION;

        A_LOG_WARNING(a_Router_LogTag, "Session %#x received invalid lease", id);
    }
    else if (lease < session->lease)
    {
        session->lease = lease;
    }

    if ((A_ERR_NONE == error) && !session->accept_sent)
    {
        a_Transport_MessageReset(&session->message);
        (void)a_Transport_MessageAccept(&session->message, session->lease);
        error = a_Router_SessionMessageSend(id, session);

        if (A_ERR_NONE == error)
        {
            session->accept_sent = true;
        }
        else
        {
            A_LOG_ERROR(a_Router_LogTag, "Session %#x failed to send accept message with error %s", id, a_Err_ToString(error));
        }
    }

    return error;
}

static a_Err_t a_Router_SessionHandlePublish(const a_Router_SessionId_t id, a_Router_Session_t *const session)
{
    a_Err_t                              error           = A_ERR_NONE;
    const a_Transport_PeerId_t           peer_id         = a_Transport_GetMessagePeerId(&session->message);
    const a_Transport_SequenceNumber_t   sequence_number = a_Transport_GetMessageSequenceNumber(&session->message);
    const a_Hash_t                       hash            = a_Transport_GetMessageKeyHash(&session->message);
    const a_Router_Subscription_t *const subscription    = a_Hashmap_Get(&a_Router_Subscriptions, &hash, sizeof(hash));

    if (NULL != subscription)
    {
        const size_t                  size               = a_Transport_GetMessageDataSize(&session->message);
        const uint8_t *const          data               = a_Transport_GetMessageData(&session->message);
        a_Router_SubscriberSession_t *subscriber_session = subscription->sessions;

        if (NULL != subscription->function)
        {
            subscription->function(subscription->key, data, size, subscription->arg);
        }

        while (NULL != subscriber_session)
        {
            if (id != subscriber_session->id)
            {
                a_Router_Session_t *forward_session = a_Hashmap_Get(&a_Router_Sessions, &subscriber_session->id, sizeof(subscriber_session->id));

                a_Transport_MessageReset(&forward_session->message);
                a_Err_t send_error = a_Transport_MessagePublish(&forward_session->message, subscription->key, data, size);

                if (A_ERR_NONE == send_error)
                {
                    (void)a_Transport_SerializeMessage(&forward_session->message, peer_id, sequence_number);

                    send_error = a_Socket_Send(&forward_session->socket, a_Transport_GetBuffer(&forward_session->message));
                }

                if (A_ERR_NONE != send_error)
                {
                    A_LOG_ERROR(a_Router_LogTag, "Session %#x sending publish message with error %s", subscriber_session->id, a_Err_ToString(send_error));
                    error = send_error;
                }
            }

            subscriber_session = subscriber_session->next;
        }
    }

    return error;
}

static a_Err_t a_Router_SessionHandleSubscribe(const a_Router_SessionId_t id, a_Router_Session_t *const session)
{
    a_Err_t           error    = A_ERR_NONE;
    const size_t      key_size = a_Transport_GetMessageKeySize(&session->message);
    const char *const key      = a_Transport_GetMessageKey(&session->message);

    if ((key_size < SIZE_MAX) && (NULL != key))
    {
        const a_Hash_t                 hash         = a_Hash_String(key, key_size);
        a_Router_Subscription_t *const subscription = a_Hashmap_Get(&a_Router_Subscriptions, &hash, sizeof(hash));

        if (NULL != subscription)
        {
            const a_Router_SubscriberSession_t *subscriber_session = subscription->sessions;

            while ((NULL != subscriber_session) && (id != subscriber_session->id))
            {
                subscriber_session = subscriber_session->next;
            }

            if (NULL == subscriber_session)
            {
                a_Router_SubscriberSession_t *const new_subscriber_session = a_malloc(sizeof(a_Router_SubscriberSession_t));
                new_subscriber_session->id   = id;
                new_subscriber_session->next = subscription->sessions;
                subscription->sessions       = new_subscriber_session;

                a_Hashmap_ForEach(&a_Router_Sessions, a_Router_SessionForwardSubscribeCallback, session);
            }
        }
        else if (a_Router_RoutingEnabled)
        {
            a_Router_Subscription_t new_subscription = {
                .sessions = a_malloc(sizeof(a_Router_SubscriberSession_t)),
                .function = NULL,
                .arg      = NULL,
                .key      = a_malloc(key_size),
            };
            new_subscription.sessions->id   = id;
            new_subscription.sessions->next = NULL;
            a_Transport_CopyString(new_subscription.key, key, key_size);

            error = a_Hashmap_Insert(&a_Router_Subscriptions, &hash, sizeof(hash), &new_subscription, sizeof(new_subscription));

            if (A_ERR_NONE == error)
            {
                a_Hashmap_ForEach(&a_Router_Sessions, a_Router_SessionForwardSubscribeCallback, session);
            }
            else
            {
                A_LOG_ERROR(a_Router_LogTag, "Session %#x failed to register subscription with error %s", id, a_Err_ToString(error));
            }
        }
    }
    else
    {
        A_LOG_WARNING(a_Router_LogTag, "Session %#x received invalid subscribe message", id);
    }

    return error;
}

static a_Err_t a_Router_SessionHandleUnsubscribe(const a_Router_SessionId_t id, a_Router_Session_t *const session)
{
    a_Err_t           error    = A_ERR_NONE;
    const size_t      key_size = a_Transport_GetMessageKeySize(&session->message);
    const char *const key      = a_Transport_GetMessageKey(&session->message);

    if ((SIZE_MAX != key_size) && (NULL != key))
    {
        const a_Hash_t                 hash         = a_Hash_String(key, key_size);
        a_Router_Subscription_t *const subscription = a_Hashmap_Get(&a_Router_Subscriptions, &hash, sizeof(hash));

        if (NULL != subscription)
        {
            error = a_Router_RemoveSubscriberSession(subscription, hash, id);
        }
    }

    return error;
}

static a_Err_t a_Router_RemoveSubscriberSession(a_Router_Subscription_t *const subscription, const a_Hash_t hash, const a_Router_SessionId_t id)
{
    a_Err_t                        error              = A_ERR_NONE;
    a_Router_SubscriberSession_t * subscriber_session = subscription->sessions;
    a_Router_SubscriberSession_t **previous           = &subscription->sessions;

    while (NULL != subscriber_session)
    {
        if (id == subscriber_session->id)
        {
            *previous = subscriber_session->next;
            a_free(subscriber_session);
            break;
        }

        previous           = &subscriber_session->next;
        subscriber_session = subscriber_session->next;
    }

    // TODO remove subscription if sessions and function are NULL
    (void)hash;
    // if ((NULL == subscription->sessions) && (NULL == subscription->function))
    // {
    //     a_free(subscription->key);
    //     error = a_Hashmap_Remove(&a_Router_Subscriptions, &hash, sizeof(hash));
    // }

    return error;
}

static void a_Router_SessionTaskCallback(const void *const key, const size_t key_size, void *const value, const size_t value_size, const void *const arg)
{
    A_UNUSED(arg);
    A_UNUSED(key_size);
    A_UNUSED(value_size);

    a_Err_t                    error   = A_ERR_NONE;
    const a_Router_SessionId_t id      = *(a_Router_SessionId_t *)key;
    a_Router_Session_t *const  session = value;

    switch (session->state)
    {
    case A_ROUTER_SESSION_STATE_CONNECT:
        error = a_Router_SessionConnect(id, session);
        break;
    case A_ROUTER_SESSION_STATE_ACCEPT:
        error = a_Router_SessionAccept(id, session);
        break;
    case A_ROUTER_SESSION_STATE_OPEN:
        error = a_Router_SessionOpen(id, session);
        break;
    case A_ROUTER_SESSION_STATE_CLOSED:
        error = a_Router_SessionClose(id, session);
        break;
    case A_ROUTER_SESSION_STATE_FAILED:
        /* Catch and handle failure cases here */
        session->state = A_ROUTER_SESSION_STATE_CLOSED;
        break;
    default:
        session->state = A_ROUTER_SESSION_STATE_FAILED;
        break;
    }

    /* TODO pass error to event handler */
    (void)error;
}

static void a_Router_SessionSubscribeCallback(const void *const key, const size_t key_size, void *const value, const size_t value_size, const void *const arg)
{
    A_UNUSED(key_size);
    A_UNUSED(value_size);

    a_Router_Session_t *const session = value;

    if (A_ROUTER_SESSION_STATE_OPEN == session->state)
    {
        a_Transport_MessageReset(&session->message);

        a_Err_t error = a_Transport_MessageSubscribe(&session->message, arg);

        if (A_ERR_NONE == error)
        {
            (void)a_Transport_SerializeMessage(&session->message, a_Router_PeerId, a_Router_SequenceNumber);

            error = a_Socket_Send(&session->socket, a_Transport_GetBuffer(&session->message));
        }

        if (A_ERR_NONE != error)
        {
            A_LOG_ERROR(a_Router_LogTag, "Session %#x sending subscribe message with error %s", *(a_Router_SessionId_t *)key, a_Err_ToString(error));
        }
    }
}

static void a_Router_SessionUnsubscribeCallback(const void *const key, const size_t key_size, void *const value, const size_t value_size, const void *const arg)
{
    A_UNUSED(key_size);
    A_UNUSED(value_size);

    a_Router_Session_t *const session = value;

    if (A_ROUTER_SESSION_STATE_OPEN == session->state)
    {
        a_Transport_MessageReset(&session->message);

        a_Err_t error = a_Transport_MessageUnsubscribe(&session->message, arg);

        if (A_ERR_NONE == error)
        {
            (void)a_Transport_SerializeMessage(&session->message, a_Router_PeerId, a_Router_SequenceNumber);

            error = a_Socket_Send(&session->socket, a_Transport_GetBuffer(&session->message));
        }

        if (A_ERR_NONE != error)
        {
            A_LOG_ERROR(a_Router_LogTag, "Session %#x sending unsubscribe message with error %s", *(a_Router_SessionId_t *)key, a_Err_ToString(error));
        }
    }
}

static void a_Router_RemoveSubscriberSessionCallback(const void *const key, const size_t key_size, void *const value, const size_t value_size, const void *const arg)
{
    A_UNUSED(key_size);
    A_UNUSED(value_size);

    (void)a_Router_RemoveSubscriberSession(value, *(a_Hash_t *)key, *(a_Router_SessionId_t *)arg);
}

static void a_Router_FreeSubscriptionCallback(const void *const key, const size_t key_size, void *const value, const size_t value_size, const void *const arg)
{
    A_UNUSED(key);
    A_UNUSED(key_size);
    A_UNUSED(value_size);
    A_UNUSED(arg);

    a_Router_Subscription_t *const subscription       = value;
    a_Router_SubscriberSession_t * subscriber_session = subscription->sessions;

    a_free(subscription->key);

    while (NULL != subscriber_session)
    {
        a_Router_SubscriberSession_t *const next = subscriber_session->next;

        a_free(subscriber_session);

        subscriber_session = next;
    }
}

static void a_Router_SessionForwardSubscribeCallback(const void *const key, const size_t key_size, void *const value, const size_t value_size, const void *const arg)
{
    A_UNUSED(key_size);
    A_UNUSED(value_size);

    const a_Router_Session_t *const session_receive = (const a_Router_Session_t *)arg;
    a_Router_Session_t *const       session         = (a_Router_Session_t *)value;

    if ((session_receive != session) && (A_ROUTER_SESSION_STATE_OPEN == session->state))
    {
        const a_Transport_PeerId_t         peer_id         = a_Transport_GetMessagePeerId(&session_receive->message);
        const a_Transport_SequenceNumber_t sequence_number = a_Transport_GetMessageSequenceNumber(&session_receive->message);
        const char *const                  key_string      = a_Transport_GetMessageKey(&session_receive->message);

        a_Transport_MessageReset(&session->message);

        a_Err_t error = a_Transport_MessageSubscribe(&session->message, key_string);

        if (A_ERR_NONE == error)
        {
            (void)a_Transport_SerializeMessage(&session->message, peer_id, sequence_number);

            error = a_Socket_Send(&session->socket, a_Transport_GetBuffer(&session->message));
        }

        if (A_ERR_NONE != error)
        {
            A_LOG_ERROR(a_Router_LogTag, "Session %#x forwarding subscribe message with error %s", *(a_Router_SessionId_t *)key, a_Err_ToString(error));
        }
    }
}

static void a_Router_SessionSendSubscriptionsCallback(const void *const key, const size_t key_size, void *const value, const size_t value_size, const void *const arg)
{
    A_UNUSED(key);
    A_UNUSED(key_size);
    A_UNUSED(value_size);

    const a_Router_Subscription_t *const subscription = (a_Router_Subscription_t *)value;

    if ((NULL != subscription->function) || (NULL != subscription->sessions))
    {
        const a_Router_SessionId_t id      = *(const a_Router_SessionId_t *)arg;
        a_Router_Session_t *const  session = a_Hashmap_Get(&a_Router_Sessions, &id, sizeof(id));

        a_Transport_MessageReset(&session->message);

        a_Err_t error = a_Transport_MessageSubscribe(&session->message, subscription->key);

        if (A_ERR_NONE == error)
        {
            a_Router_SerializeMessage(&session->message);

            error = a_Socket_Send(&session->socket, a_Transport_GetBuffer(&session->message));
        }

        if (A_ERR_NONE != error)
        {
            A_LOG_ERROR(a_Router_LogTag, "Session %#x sending initial subscribe message with error %s", subscription->key, a_Err_ToString(error));
        }

        (void)a_Router_SessionOpen(id, session);
    }
}
