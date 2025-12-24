#include "leb128.h"

#include <stddef.h>
#include <stdint.h>

#define LEB128_7_BIT_SHIFT         7U
#define LEB128_LOWER_7_BITS_MASK   0x7FU
#define LEB128_HIGH_ORDER_BIT_MASK 0x80U

static size_t Leb128_Decode(uint64_t *const value, const size_t max_bytes, const uint8_t *const buffer, const size_t size);

size_t Leb128_Encode8(uint8_t value, uint8_t *const buffer, const size_t size)
{
    return Leb128_Encode64((uint64_t)value, buffer, size);
}

size_t Leb128_Encode16(uint16_t value, uint8_t *const buffer, const size_t size)
{
    return Leb128_Encode64((uint64_t)value, buffer, size);
}

size_t Leb128_Encode32(uint32_t value, uint8_t *const buffer, const size_t size)
{
    return Leb128_Encode64((uint64_t)value, buffer, size);
}

size_t Leb128_Encode64(uint64_t value, uint8_t *const buffer, const size_t size)
{
    size_t encoded = SIZE_MAX;

    if ((NULL != buffer) && (0U != size))
    {
        size_t byte = 0U;

        while (byte < size)
        {
            *(buffer + byte) = (uint8_t)(value & LEB128_LOWER_7_BITS_MASK);
            value          >>= LEB128_7_BIT_SHIFT;

            if (0U == value)
            {
                encoded = ++byte;
                break;
            }
            else
            {
                *(buffer + byte++) |= LEB128_HIGH_ORDER_BIT_MASK;
            }
        }
    }

    return encoded;
}

size_t Leb128_Decode8(uint8_t *const value, const uint8_t *const buffer, const size_t size)
{
    size_t decoded = SIZE_MAX;

    if (NULL != value)
    {
        uint64_t u64 = 0U;
        decoded = Leb128_Decode(&u64, LEB128_MAX_SIZE(uint8_t), buffer, size);
        *value  = (uint8_t)u64;
    }

    return decoded;
}

size_t Leb128_Decode16(uint16_t *const value, const uint8_t *const buffer, const size_t size)
{
    size_t decoded = SIZE_MAX;

    if (NULL != value)
    {
        uint64_t u64 = 0U;
        decoded = Leb128_Decode(&u64, LEB128_MAX_SIZE(uint16_t), buffer, size);
        *value  = (uint16_t)u64;
    }

    return decoded;
}

size_t Leb128_Decode32(uint32_t *const value, const uint8_t *const buffer, const size_t size)
{
    size_t decoded = SIZE_MAX;

    if (NULL != value)
    {
        uint64_t u64 = 0U;
        decoded = Leb128_Decode(&u64, LEB128_MAX_SIZE(uint32_t), buffer, size);
        *value  = (uint32_t)u64;
    }

    return decoded;
}

size_t Leb128_Decode64(uint64_t *const value, const uint8_t *const buffer, const size_t size)
{
    return Leb128_Decode(value, LEB128_MAX_SIZE(uint64_t), buffer, size);
}

static size_t Leb128_Decode(uint64_t *const value, const size_t max_bytes, const uint8_t *const buffer, const size_t size)
{
    size_t decoded = SIZE_MAX;

    if ((NULL != value) && (NULL != buffer) && (0U != size))
    {
        size_t byte  = 0U;
        size_t shift = 0U;
        *value = 0U;

        while ((byte < size) && (byte < max_bytes))
        {
            *value |= (uint64_t)((*(buffer + byte)) & LEB128_LOWER_7_BITS_MASK) << shift;

            if (0U == (*(buffer + byte) & LEB128_HIGH_ORDER_BIT_MASK))
            {
                decoded = ++byte;
                break;
            }
            else
            {
                shift += LEB128_7_BIT_SHIFT;
                byte++;
            }
        }
    }

    return decoded;
}
