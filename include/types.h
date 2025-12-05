#ifndef AETHER_TYPES_H
#define AETHER_TYPES_H

#include <stddef.h>
#include <stdint.h>

#define A_UNUSED(unused) (void)(unused)

typedef uint32_t a_PeerId_t;
typedef uint32_t a_SessionId_t;
typedef uint64_t a_SequenceNumber_t;
typedef uint64_t a_Milliseconds_t;

typedef enum
{
    A_ERROR_NONE,
    A_ERROR_NULL,
    A_ERROR_SIZE,
    A_ERROR_SOCKET
} a_Error_t;

typedef enum
{
    A_HEADER_OPEN,
    A_HEADER_ACCEPT,
    A_HEADER_CLOSE,
    A_HEADER_RENEW,
    A_HEADER_SUBSCRIBE,
    A_HEADER_PUBLISH
} a_Header_t;

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/* TODO error to string */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AETHER_TYPES_H */
