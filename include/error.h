#ifndef AETHER_ERROR_H
#define AETHER_ERROR_H

#include <stddef.h>
#include <stdint.h>

/* TODO move to permanent location */
#define A_UNUSED(unused) (void)(unused)

typedef enum
{
    A_ERROR_NONE,
    A_ERROR_NULL,
    A_ERROR_SIZE,
    A_ERROR_SOCKET
} a_Error_t;

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/* TODO error to string */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AETHER_ERROR_H */
