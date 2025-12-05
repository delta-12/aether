#ifndef AETHER_COBS_H
#define AETHER_COBS_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

size_t Cobs_Encode(const void *const data, const size_t data_size, uint8_t *const buffer, const size_t buffer_size);
size_t Cobs_Decode(void *const data, const size_t data_size, const uint8_t *const buffer, const size_t buffer_size);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AETHER_COBS_H */