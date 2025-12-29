#include <chrono>
#include <thread>

#include <gtest/gtest.h>

#include "tick.h"

TEST(Tick, GetTick)
{
    a_Tick_Ms_t tick = a_Tick_GetTick();

    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    ASSERT_GE(a_Tick_GetTick(), ++tick);

    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    ASSERT_GE(a_Tick_GetTick(), ++tick);
}

TEST(Tick, GetElapsed)
{
    a_Tick_Ms_t tick = a_Tick_GetTick();

    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    ASSERT_GE(5U, a_Tick_GetElapsed(tick));
}
