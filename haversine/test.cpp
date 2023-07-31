#include <unistd.h>

#include "profiler.cpp"

void b()
{
   TimeFunction;
   usleep(1000);
}

void a()
{
   TimeFunction;
   usleep(1000);
   b();
}

void c(int i)
{
   TimeFunction;
   if (i == 0)
      return;
   usleep(1000);
   c(i-1);
}

void d()
{
   TimeFunction;
   c(2);
}

void e(int i);
void f(int i);

void e(int i)
{
   TimeFunction;
   if (i == 0)
      return;
   usleep(1000);
   f(i-1);
}

void f(int i)
{
   TimeFunction;
   if (i == 0)
      return;
   b();
   usleep(1000);
   e(i-1);
}

int main()
{
   startprofiler();
   a();
   d();
   c(10);
   f(10);
   stopprofiler();

   print_zone_statistics();

   return 0;
}
