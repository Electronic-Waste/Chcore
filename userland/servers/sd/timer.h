#pragma once
#include "sd.h"

#define LONG_MAX 9223372036854775807L
#define time_t u64
struct timespec {
	time_t tv_sec; /* Seconds. */
	time_t tv_nsec; /* Nanoseconds. */
};

void SimpleMsDelay(unsigned nMilliSeconds);
void SimpleusDelay(unsigned nMicroSeconds);
inline void timer_usDelay(unsigned nMicroSeconds)
{
	SimpleusDelay(nMicroSeconds);
}

long clock();

#if defined (USE_PHYSICAL_COUNTER) && AARCH == 64
	// u32  m_nClockTicksPerHZTick;
#endif
