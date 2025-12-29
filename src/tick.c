#include "tick.h"

#ifndef AETHER_TICK_USER
#include <time.h>

#define A_TICK_MILLISECONDS_TO_SECONDS     1000U
#define A_TICK_NANOSECONDS_TO_MILLISECONDS 1000000U
#else
extern a_Tick_Ms_t a_TickUser_GetTick(void);
#endif /* AETHER_TICK_USER */

a_Tick_Ms_t a_Tick_GetTick(void)
{
#ifdef AETHER_TICK_USER
    return a_TickUser_GetTick();
#else
    a_Tick_Ms_t     tick = A_TICK_MS_MAX;
    struct timespec time;

    if (0 != timespec_get(&time, TIME_UTC))
    {
        tick = (a_Tick_Ms_t)((int64_t)time.tv_sec * A_TICK_MILLISECONDS_TO_SECONDS + (int64_t)time.tv_nsec / A_TICK_NANOSECONDS_TO_MILLISECONDS);
    }

    return tick;
#endif /* AETHER_TICK_USER */
}

a_Tick_Ms_t a_Tick_GetElapsed(const a_Tick_Ms_t start)
{
    return a_Tick_GetTick() - start;
}
