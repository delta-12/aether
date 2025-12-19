#include "session.h"

#include "err.h"
#include "router.h"
#include "socket.h"
#include "transport.h"

static a_Err_t a_Session_Connect(Session_t *const session);
static a_Err_t a_Session_Accept(Session_t *const session);
static a_Err_t a_Session_Open(Session_t *const session);

a_Err_t a_Session_Initialize(Session_t *const session, a_Socket_t *const socket, uint8_t *const buffer, const size_t buffer_size)
{
    a_Err_t error = A_ERR_NULL;

    if ((NULL != session) && (NULL != socket))
    {
        session->state       = A_SESSION_STATE_CONNECT;
        session->lease       = 1000U; /* TODO */
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

        if ((A_ERR_NONE != error) || (0U != a_Buffer_GetReadSize(a_Transport_GetMessageBuffer(&message))))
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
            session->state = A_SESSION_STATE_OPEN;
        }
        else
        {
            error = A_ERR_SEQUENCE;
        }
    }

    if (A_ERR_NONE != error)
    {
        /* TODO log error and session failure */
        session->state = A_SESSION_STATE_FAILED;
    }

    return error;
}

static a_Err_t a_Session_Open(Session_t *const session)
{
    A_UNUSED(session);

    /* TODO */

    /* TODO handle lease expiration */

    return A_ERR_NONE;
}
