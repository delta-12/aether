#include "session.h"

#include "err.h"
#include "socket.h"
#include "transport.h"

static a_Transport_PeerId_t peer_id = 0U;

static a_Transport_SequenceNumber_t a_Session_GetSequenceNumber(void);
static a_Err_t a_Session_SendMessage(Session_t *const session, a_Transport_Message_t *const message);
static a_Err_t a_Session_Connect(Session_t *const session);
static a_Err_t a_Session_Accept(Session_t *const session);
static a_Err_t a_Session_Open(Session_t *const session);

void a_Session_SetPeerId(const a_Transport_PeerId_t *const id)
{
    if (NULL == id)
    {
        peer_id = 0U; /* TODO randomly generate */
    }
    else
    {
        peer_id = *id;
    }

    /* TODO log peer id */
}

a_Err_t a_Session_Initialize(Session_t *const session,
                             const a_Transport_SessionId_t id,
                             a_Socket_t *const socket,
                             uint8_t *const send_buffer,
                             const size_t send_buffer_size,
                             uint8_t *const receive_buffer,
                             const size_t receive_buffer_size)
{
    a_Err_t error = A_ERR_NULL;

    if ((NULL != session) && (NULL != socket))
    {
        session->id                  = id;
        session->socket              = *socket;
        session->state               = A_SESSION_STATE_CONNECT; /* TODO */
        session->lease               = 1000U;                   /* TODO */
        session->send_buffer         = send_buffer;
        session->send_buffer_size    = send_buffer_size;
        session->receive_buffer      = receive_buffer;
        session->receive_buffer_size = receive_buffer_size;

        /* TODO log new session with ID and state */

        error = A_ERR_NONE;
    }

    return error;
}

a_Err_t a_Session_GetState(Session_t *const session, a_Session_State_t *const state)
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

static a_Transport_SequenceNumber_t a_Session_GetSequenceNumber(void)
{
    static a_Transport_SequenceNumber_t a_sequence_number = 0U;

    return a_sequence_number++;
}

static a_Err_t a_Session_SendMessage(Session_t *const session, a_Transport_Message_t *const message)
{
    a_Buffer_t *buffer = a_Transport_SerializeMessage(message);
    a_Err_t     error  = A_ERR_SERIALIZE;

    if (NULL == buffer)
    {
        /* TODO log error */
    }
    else
    {
        while (a_Buffer_GetReadSize(buffer) > 0U)
        {
            error = a_Socket_Send(&session->socket, buffer);

            if (A_ERR_NONE != error)
            {
                /* TODO log error */
                break;
            }
        }
    }

    return error;
}

static a_Err_t a_Session_Connect(Session_t *const session)
{
    a_Transport_Message_t message;
    a_Err_t               error = a_Transport_MessageInitialize(&message, peer_id, a_Session_GetSequenceNumber(), session->send_buffer, session->send_buffer_size);

    if (A_ERR_NONE == error)
    {
        error = a_Transport_MessageConnect(&message, session->id, session->lease);
    }

    if (A_ERR_NONE == error)
    {
        error = a_Session_SendMessage(session, &message);
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
    A_UNUSED(session);

    /* TODO */

    return A_ERR_NONE;
}

static a_Err_t a_Session_Open(Session_t *const session)
{
    A_UNUSED(session);

    /* TODO */

    return A_ERR_NONE;
}
