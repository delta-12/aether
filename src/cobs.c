#include "cobs.h"

#include <stddef.h>
#include <stdint.h>

size_t Cobs_Encode(const void *const data, const size_t data_size, uint8_t *const buffer, const size_t buffer_size)
{
    size_t encoded = 1U;

    if ((NULL == data) || (NULL == buffer))
    {
        encoded = SIZE_MAX;
    }
    else
    {
        size_t data_index = 0U;
        size_t code_index = 0U;
        uint8_t code = 1U;

        while ((data_index < data_size) && (encoded < buffer_size))
        {
            uint8_t byte = *((uint8_t *)data + data_index);

            if ((0U == byte) || (UINT8_MAX == code))
            {
                if (UINT8_MAX != code)
                {
                    data_index++;
                }

                *(buffer + code_index) = code;
                code = 1U;
                code_index = encoded;
                encoded++; 
            }
            else
            {
                *(buffer + encoded) = byte;
                encoded++;
                data_index++;
                code++;
            }
        }

        if (encoded < buffer_size)
        {
            *(buffer + code_index) = code;
            *(buffer + encoded) = 0U;
            encoded++;
        }
        else
        {
            encoded = SIZE_MAX;
        }
    }

    return encoded;
}

size_t Cobs_Decode(void *const data, const size_t data_size, const uint8_t *const buffer, const size_t buffer_size)
{
    size_t decoded = 0U;

    if ((NULL == data) || (NULL == buffer))
    {
        decoded = SIZE_MAX;
    }
    else
    {
        (void)data;
        (void)data_size;
        (void)buffer;
        (void)buffer_size;
    }

    return decoded;
}