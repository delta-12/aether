#ifndef AETHER_ERROR_H
#define AETHER_ERROR_H

#include <stddef.h>
#include <stdint.h>

/* TODO move to permanent location */
#define A_UNUSED(unused) (void)(unused)

typedef enum
{
    A_ERR_NONE,
    A_ERR_NULL,
    A_ERR_SIZE,
    A_ERR_SERIALIZATION,
    A_ERR_SOCKET,
    A_ERR_SEQUENCE,
    A_ERR_MAX
} a_Err_t;

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

char *a_Err_ToString(const a_Err_t err);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AETHER_ERROR_H */
