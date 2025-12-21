#ifndef AETHER_TICK_H
#define AETHER_TICK_H

#include <stdint.h>

typedef uint64_t a_Tick_Ms_t;

#define A_TICK_MS_MAX (a_Tick_Ms_t)(UINT64_MAX)

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

a_Tick_Ms_t a_Tick_GetTick(void);
a_Tick_Ms_t a_Tick_GetElapsed(const a_Tick_Ms_t start, const a_Tick_Ms_t end);
a_Tick_Ms_t a_Tick_GetElapsedNow(const a_Tick_Ms_t start);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AETHER_TICK_H */
