#ifndef AETHER_LEB128_H
#define AETHER_LEB128_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

size_t Leb128_Encode8(uint8_t value, uint8_t *const buffer, const size_t size);
size_t Leb128_Encode16(uint16_t value, uint8_t *const buffer, const size_t size);
size_t Leb128_Encode32(uint32_t value, uint8_t *const buffer, const size_t size);
size_t Leb128_Encode64(uint64_t value, uint8_t *const buffer, const size_t size);
size_t Leb128_Decode8(uint8_t *const value, const uint8_t *const buffer, const size_t size);
size_t Leb128_Decode16(uint16_t *const value, const uint8_t *const buffer, const size_t size);
size_t Leb128_Decode32(uint32_t *const value, const uint8_t *const buffer, const size_t size);
size_t Leb128_Decode64(uint64_t *const value, const uint8_t *const buffer, const size_t size);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ARTHER_LEB128_h */
