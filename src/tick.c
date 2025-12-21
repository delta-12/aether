#include "tick.h"

a_Tick_Ms_t a_Tick_GetTick(void)
{
    /* TODO */
    return 0U;
}

a_Tick_Ms_t a_Tick_GetElapsed(const a_Tick_Ms_t start, const a_Tick_Ms_t end)
{
    return end - start;
}

a_Tick_Ms_t a_Tick_GetElapsedNow(const a_Tick_Ms_t start)
{
    return a_Tick_GetElapsed(start, a_Tick_GetTick());
}
