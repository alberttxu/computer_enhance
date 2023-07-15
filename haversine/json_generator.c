#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

#include "listing65.c"

double sum = 0;

double randfloat64(double minval, double maxval)
{
   return (double)rand() / (double)(RAND_MAX) * (maxval - minval) + minval;
}

void write_entry(FILE* f)
{
   double x0 = randfloat64(-180, 180);
   double y0 = randfloat64(-90, 90);
   double x1 = randfloat64(-180, 180);
   double y1 = randfloat64(-90, 90);
   fprintf(f, "{\"x0\":%f, \"y0\":%f, \"x1\":%f, \"y1\":%f}", x0, y0, x1, y1);

   sum += ReferenceHaversine(x0, y0, x1, y1, 6372.8);
}

int main(int argc, char** argv)
{
   if (argc != 3)
   {
      fprintf(stderr, "usage: $ ./generator random_seed num_entries\n");
      return 1;
   }
   int seed = atoi(argv[1]);
   int num_entries = atoi(argv[2]);

   FILE* outputfile = fopen("haversine_data.json", "w");
   assert(outputfile);

   fprintf(outputfile, "{\"pairs\":[\n");
   srand(seed);
   for (size_t i = 0; i < num_entries; i++)
   {
      write_entry(outputfile);

      if (i < num_entries - 1)
         fprintf(outputfile, ",");
      fprintf(outputfile, "\n");
   }
   fprintf(outputfile, "]}");

   printf("# of entries: %d\n", num_entries);
   printf("random seed: %d\n", seed);
   printf("avg: %f\n", sum / num_entries);

   fclose(outputfile);
   return 0;
}
