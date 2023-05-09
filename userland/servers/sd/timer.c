#include <stdio.h>
// #include <sched.h>
// #include <time.h>
#include <sys/types.h>
#include <chcore/internal/raw_syscall.h>
#include "timer.h"

long clock()
{
	struct timespec ts;

	if (__chcore_sys_clock_gettime(2 /* CLOCK_PROCESS_CPUTIME_ID */, (u64)&ts))
		return -1;

	if (ts.tv_sec > LONG_MAX/1000000
	 || ts.tv_nsec/1000 > LONG_MAX-1000000*ts.tv_sec)
		return -1;

	return ts.tv_sec*1000000 + ts.tv_nsec/1000;
}

void SimpleMsDelay(unsigned nMilliSeconds)
{
	if (nMilliSeconds > 0) {
		SimpleusDelay(nMilliSeconds * 1000);
	}
}

void SimpleusDelay(unsigned nMicroSeconds)
{
	if (nMicroSeconds > 0) {
		int start_time = clock();
		while (clock() - start_time < nMicroSeconds) {
			// do nothing
			__chcore_sys_yield();
		}
	}
}
