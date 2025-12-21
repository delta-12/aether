#include "session.h"

#include "err.h"
#include "router.h"
#include "socket.h"
#include "transport.h"

#define A_SESSION_LEASE_TOLERANCE (a_Tick_Ms_t)10U   /* TODO make sure tolerance is not greater than minimum lease time or make tolerance a percentage */
#define A_SESSION_DEFAULT_LEASE   (a_Tick_Ms_t)1000U /* TODO */

static a_Err_t a_Session_Connect(Session_t *const session);
static a_Err_t a_Session_Accept(Session_t *const session);
static a_Err_t a_Session_Open(Session_t *const session);

a_Err_t a_Session_Initialize(Session_t *const session, a_Socket_t *const socket, uint8_t *const buffer, const size_t buffer_size)
{
    a_Err_t error = A_ERR_NULL;

    if ((NULL != session) && (NULL != socket))
    {
        session->state       = A_SESSION_STATE_CONNECT;
        session->lease       = A_SESSION_DEFAULT_LEASE;
        session->buffer      = buffer;
        session->buffer_size = buffer_size;

        /* TODO log new session with ID and state */

        error = a_Router_SessionAdd(session->id, socket);
    }

    return error;
}

a_Err_t a_Session_GetState(const Session_t *const session, a_Session_State_t *const state)
{
    a_Err_t error = A_ERR_NULL;

    if ((NULL != session) && (NULL != state))
    {
        *state = session->state;
        error  = A_ERR_NONE;
    }

    return error;
}

a_Err_t a_Session_Task(Session_t *const session)
{
    a_Err_t error = A_ERR_NONE;

    switch (session->state)
    {
    case A_SESSION_STATE_CONNECT:
        error = a_Session_Connect(session);
        break;
    case A_SESSION_STATE_ACCEPT:
        error = a_Session_Accept(session);
        break;
    case A_SESSION_STATE_OPEN:
        error = a_Session_Open(session);
        break;
    case A_SESSION_STATE_CLOSED:
        /* TODO */
        break;
    case A_SESSION_STATE_FAILED:
        /* TODO */
        break;
    default:
        /* TODO error handling */
        break;
    }

    return error;
}

static a_Err_t a_Session_Connect(Session_t *const session)
{
    a_Transport_Message_t message;
    a_Err_t               error = a_Transport_MessageInitialize(&message, session->buffer, session->buffer_size);

    if (A_ERR_NONE == error)
    {
        (void)a_Transport_MessageConnect(&message, session->lease);
        error = a_Router_SessionMessageSend(session->id, &message);
    }

    if (A_ERR_NONE == error)
    {
        /* TODO log */
        session->state = A_SESSION_STATE_ACCEPT;
    }
    else
    {
        /* TODO log */
        session->state = A_SESSION_STATE_FAILED;
    }

    return error;
}

static a_Err_t a_Session_Accept(Session_t *const session)
{
    a_Transport_Message_t message;
    a_Err_t               error = a_Transport_MessageInitialize(&message, session->buffer, session->buffer_size);

    /* TODO timeout and retries */

    if (A_ERR_NONE == error)
    {
        error = a_Router_SessionMessageGet(session->id, &message);

        if ((A_ERR_NONE != error) || !a_Transport_IsMessageDeserialized(&message))
        {
            /* Error receiving message or no message received */
        }
        else if (A_TRANSPORT_HEADER_CONNECT == a_Transport_GetMessageHeader(&message))
        {
            /* TODO handle version mismatch and arbitrate MTU, make sure to get fields in correct order */
            a_Tick_Ms_t lease = a_Transport_GetMessageLease(&message);
            if (lease < session->lease)
            {
                session->lease = lease;
            }

            (void)a_Transport_MessageAccept(&message, session->lease);
            error = a_Router_SessionMessageSend(session->id, &message);
        }
        else if ((A_TRANSPORT_HEADER_ACCEPT == a_Transport_GetMessageHeader(&message)) && (a_Transport_GetMessageLease(&message) == session->lease))
        {
            /* TODO verify MTU matches */
            /* TODO log session open */

            a_Tick_Ms_t now = a_Tick_GetTick();
            session->last_renew_received = now;
            session->last_renew_sent     = now;
            session->state               = A_SESSION_STATE_OPEN;
        }
        else
        {
            error = A_ERR_SEQUENCE;
        }
    }

    if (A_ERR_NONE != error)
    {
        /* TODO log session failure */
        session->state = A_SESSION_STATE_FAILED;
    }

    return error;
}

static a_Err_t a_Session_Open(Session_t *const session)
{
    a_Transport_Message_t message;
    a_Err_t               error = a_Transport_MessageInitialize(&message, session->buffer, session->buffer_size);

    if (A_ERR_NONE == error)
    {
        a_Err_t error = a_Router_SessionMessageGet(session->id, &message);

        if (A_ERR_NONE != error)
        {
            /* Error receiving message */
        }
        else if (a_Transport_IsMessageDeserialized(&message))
        {
            switch (a_Transport_GetMessageHeader(&message))
            {
            case A_TRANSPORT_HEADER_CLOSE:
                /* TODO log session close */
                session->state = A_SESSION_STATE_CLOSED;
                break;
            case A_TRANSPORT_HEADER_RENEW:
                session->last_renew_received = a_Tick_GetTick();
                break;
            case A_TRANSPORT_HEADER_SUBSCRIBE:
                /* TODO */
                break;
            case A_TRANSPORT_HEADER_PUBLISH:
                /* TODO */
                break;
            case A_TRANSPORT_HEADER_CONNECT:
            case A_TRANSPORT_HEADER_ACCEPT:
            default:
                /* TODO verbose log invalid header */
                break;
            }
        }

        if (a_Tick_GetElapsedNow(session->last_renew_received) >= (session->lease + A_SESSION_LEASE_TOLERANCE))
        {
            /* TODO log timeout error */
            session->state = A_SESSION_STATE_FAILED;
            error          = A_ERR_TIMEOUT;
        }

        if (a_Tick_GetElapsedNow(session->last_renew_sent) >= (session->lease - A_SESSION_LEASE_TOLERANCE))
        {
            (void)a_Transport_MessageInitialize(&message, session->buffer, session->buffer_size);
            (void)a_Transport_MessageRenew(&message);
            error = a_Router_SessionMessageSend(session->id, &message);
        }
    }

    return error;
}
