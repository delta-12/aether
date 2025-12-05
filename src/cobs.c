#include "cobs.h"

#include <stddef.h>
#include <stdint.h>

size_t Cobs_Encode(const void *const data, const size_t data_size, uint8_t *const buffer, const size_t buffer_size)
{
    size_t encoded = 0U;

    if ((NULL == data) || (NULL == buffer))
    {
        encoded = SIZE_MAX;
    }
    else
    {
        size_t  data_index = 0U;
        size_t  code_index = 0U;
        uint8_t code       = 1U;

        encoded = 1U;

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
                code                   = 1U;
                code_index             = encoded;
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
            *(buffer + encoded)    = 0U;
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
    size_t decoded = SIZE_MAX;

    if ((NULL != data) && (NULL != buffer) && (buffer_size > 1U))
    {
        size_t  data_index   = 0U;
        size_t  buffer_index = 1U;
        uint8_t code         = *buffer;
        size_t  block_end    = buffer_index + code - 1U;

        while ((data_index < data_size) && (buffer_index < buffer_size))
        {
            if (buffer_index < block_end)
            {
                *((uint8_t *)data + data_index) = *(buffer + buffer_index);
                data_index++;
                buffer_index++;
            }
            else
            {
                uint8_t byte = *(buffer + buffer_index);
                block_end = buffer_index + byte;
                buffer_index++;

                if (0U == byte)
                {
                    decoded = data_index;
                    break;
                }
                else if (UINT8_MAX != code)
                {
                    *((uint8_t *)data + data_index) = 0U;
                    data_index++;
                }

                code = byte;
            }
        }

        if ((SIZE_MAX == decoded) && (buffer_index < buffer_size) && (0U == *(buffer + buffer_index)))
        {
            decoded = data_index;
        }
    }

    return decoded;
}