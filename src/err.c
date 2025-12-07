#include "err.h"

char *a_Err_ToString(const a_Err_t err)
{
    char *string = "ERR_UNKNOWN";

    switch (err)
    {
    case A_ERR_NONE:
        string = "ERR_NONE";
        break;
    case A_ERR_NULL:
        string = "ERR_NULL";
        break;
    case A_ERR_SIZE:
        string = "ERR_SIZE";
        break;
    case A_ERR_SERIALIZATION:
        string = "ERR_SERIALIZATION";
        break;
    case A_ERR_SOCKET:
        string = "ERR_SOCKET";
        break;
    case A_ERR_SEQUENCE:
        string = "ERR_SEQUENCE";
        break;
    default:
        break;
    }

    return string;
}
