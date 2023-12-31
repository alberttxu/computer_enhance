#pragma once

#include <stdint.h>
#include <stdio.h>
#include <sys/time.h>
#include <assert.h>

typedef uint64_t u64;
typedef double f64;

static inline
u64 GetOSTimerFreq(void)
{
	return 1000000;
}

static inline
u64 ReadOSTimer(void)
{
	// NOTE(casey): The "struct" keyword is not necessary here when compiling in C++,
	// but just in case anyone is using this file from C, I include it.
	struct timeval Value;
	gettimeofday(&Value, 0);

	u64 Result = GetOSTimerFreq()*(u64)Value.tv_sec + (u64)Value.tv_usec;
	return Result;
}

static inline
u64 ReadCPUTimer(void)
{
	return __rdtsc();
}

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

static u64 t_profilerstart;
static u64 t_profilerend;

static inline
void startprofiler()
{
   t_profilerstart = ReadCPUTimer();
}

static inline
void stopprofiler()
{
   t_profilerend = ReadCPUTimer();
}

#ifdef ENABLE_PROFILER

struct Zone
{
   const char *name;
   u64 exclusive_time;
   u64 total_time;
   int hitcount;
   int activecount;
};

constexpr int maxzones = 20;
static Zone zones[maxzones];

constexpr int maxstacksize = 100;
static int activezones[maxstacksize];
static int numactivezones = 0;

static inline
void pushzone(int zoneid)
{
   assert(numactivezones < maxstacksize);
   activezones[numactivezones] = zoneid;
   numactivezones += 1;
}

static inline
void popzone()
{
   assert(numactivezones > 0);
   numactivezones -= 1;
}

struct ZoneTiming
{
   int zoneidx;
   u64 t_start;
   u64 t_end;

   ZoneTiming(int id, const char *name)
   {
      zoneidx = id;
      pushzone(id);
      Zone *curzone = &zones[id];
      curzone->name = name;
      curzone->hitcount += 1;
      curzone->activecount += 1;
      t_start = ReadCPUTimer();
   }

   ~ZoneTiming()
   {
      t_end = ReadCPUTimer();
      u64 duration = t_end - t_start;
      Zone *curzone = &zones[zoneidx];

      if (curzone->activecount == 1)
      {
         curzone->total_time += duration;
      }

      curzone->exclusive_time += duration; // children will do subtraction
      if (numactivezones > 1)
      {
         int parentid = activezones[numactivezones - 2];
         Zone *parent = &zones[parentid];
         parent->exclusive_time -= duration;
      }

      curzone->activecount -= 1;
      popzone();
   }
};

#define CONCAT_IMPL( x, y ) x##y
#define MACRO_CONCAT( x, y ) CONCAT_IMPL( x, y )

#define CreateZoneTiming(id, name) ZoneTiming MACRO_CONCAT(zonetiming, id)(id, name)
#define TimeBlock(name) CreateZoneTiming(__COUNTER__, name)
#define TimeFunction TimeBlock(__func__)

#else

#define TimeBlock(name)
#define TimeFunction

#endif

static inline
void print_zone_statistics()
{
   puts("==== profile results ====");
   u64 cpufreq = getcpufreq();
   printf("Total time: %f s\n", (f64)(t_profilerend - t_profilerstart) / cpufreq);
#ifdef ENABLE_PROFILER
   for (int i = 0; i < maxzones; i += 1)
   {
      if (zones[i].hitcount == 0)
         continue;

      printf("%10d  %-18s: %10llu (%.2f %%",
            zones[i].hitcount,
            zones[i].name,
            zones[i].exclusive_time,
            100.0 * (f64)zones[i].exclusive_time / (t_profilerend - t_profilerstart)
            );
      if (zones[i].exclusive_time != zones[i].total_time)
      {
         printf(", %.2f %% w/subzones",
            100.0 * (f64)zones[i].total_time / (t_profilerend - t_profilerstart));
      }
      printf(")\n");
   }
#endif
}
