#include <assert.h>
#include "my_listing74.cpp"

struct Measurement
{
   const char *funcname;
   u64 duration;
};

constexpr int arrmaxlen = 20;
Measurement measurements[arrmaxlen];
int num_measurements = 0;

void push_measurement(Measurement m)
{
   assert(num_measurements < arrmaxlen);
   measurements[num_measurements] = m;
   num_measurements += 1;
}

struct TimingMeasurement
{
   const char *funcname;
   u64 start;
   u64 end;

   TimingMeasurement(const char *funcname_)
   {
      funcname = funcname_;
      start = ReadCPUTimer();
   }

   ~TimingMeasurement()
   {
      end = ReadCPUTimer();
      Measurement m = {funcname, end - start};
      push_measurement(m);
   }
};

#define TimeFunction TimingMeasurement temp(__func__);
#define TimeBlock(name) TimingMeasurement temp(name);


void print_measurements()
{
   puts("==== profile results ====");
   for (int i = 0; i < num_measurements; i += 1)
   {
      printf("%s: %llu clocks, %f s\n",
            measurements[i].funcname,
            measurements[i].duration,
            (f64)measurements[i].duration / getcpufreq());
   }
}
