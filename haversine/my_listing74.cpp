#pragma once

#include <stdint.h>
typedef uint64_t u64;
typedef double f64;

#include "listing_0070_platform_metrics.cpp"

static inline
u64 getcpufreq(void)
{
	u64 MillisecondsToWait = 10;

	u64 OSFreq = GetOSTimerFreq();
	u64 CPUStart = ReadCPUTimer();
	u64 OSStart = ReadOSTimer();
	u64 OSEnd = 0;
	u64 OSElapsed = 0;
	u64 OSWaitTime = OSFreq * MillisecondsToWait / 1000;
	while(OSElapsed < OSWaitTime)
	{
		OSEnd = ReadOSTimer();
		OSElapsed = OSEnd - OSStart;
	}

	u64 CPUEnd = ReadCPUTimer();
	u64 CPUElapsed = CPUEnd - CPUStart;
	u64 CPUFreq = 0;
	if(OSElapsed)
	{
		CPUFreq = OSFreq * CPUElapsed / OSElapsed;
	}
   return CPUFreq;
}

static inline
f64 elapsedtime(u64 cpustart, u64 cpuend)
{
   return (f64)(cpuend - cpustart) / getcpufreq();
}

static inline
u64 elapsedticks(u64 cpustart, u64 cpuend)
{
   return cpuend - cpustart;
}
