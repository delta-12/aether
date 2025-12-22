#include <gtest/gtest.h>

#include "err.h"

TEST(Err, ErrToString)
{
    ASSERT_STREQ("ERR_NONE", a_Err_ToString(A_ERR_NONE));
    ASSERT_STREQ("ERR_NULL", a_Err_ToString(A_ERR_NULL));
    ASSERT_STREQ("ERR_SIZE", a_Err_ToString(A_ERR_SIZE));
    ASSERT_STREQ("ERR_SERIALIZATION", a_Err_ToString(A_ERR_SERIALIZATION));
    ASSERT_STREQ("ERR_SOCKET", a_Err_ToString(A_ERR_SOCKET));
    ASSERT_STREQ("ERR_SEQUENCE", a_Err_ToString(A_ERR_SEQUENCE));
    ASSERT_STREQ("ERR_DUPLICATE", a_Err_ToString(A_ERR_DUPLICATE));
    ASSERT_STREQ("ERR_TIMEOUT", a_Err_ToString(A_ERR_TIMEOUT));
    ASSERT_STREQ("ERR_UNKNOWN", a_Err_ToString(A_ERR_MAX));
}
