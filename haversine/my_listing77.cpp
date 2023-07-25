#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "listing65.c"
#include "my_listing76_simple_profiler.cpp"

#define MAXTOKENSIZE 30

typedef struct {
   int size;
   char data[MAXTOKENSIZE];
} Token;

Token newToken()
{
   Token tk;
   tk.size = 0;
   return tk;
}

void pushchar(Token* tk, char c)
{
   assert(tk->size < MAXTOKENSIZE);
   tk->data[tk->size] = c;
   tk->size += 1;
}

void printToken(Token tk)
{
   for (size_t i = 0; i < tk.size; i++)
   {
      printf("%c", tk.data[i]);
   }
}

Token readtoken(FILE* f, char until)
{
   Token tk = newToken();

   char c;
   while (fread(&c, 1, 1, f))
   {
      if (c == until || c == '}')
         break;
      pushchar(&tk, c);
   }
   return tk;
}

double parse_jsonfile(FILE* f)
{
   TimeFunction;

   char c;
   bool arraystart = false;

   int num_entries = 0;
   double sum = 0;
   double x0;
   double y0;
   double x1;
   double y1;

   for (int count = 0; fread(&c, 1, 1, f); count += 1)
   {
      if (arraystart == false)
      {
         if (c == '[')
            arraystart = true;
         continue;
      }

      if (c == '"')
      {
         Token key = readtoken(f, '"');

         fread(&c, 1, 1, f);
         assert(c == ':');
         count += 1;

         Token val = readtoken(f, ',');
         pushchar(&val, '\0');

#if 0
         printToken(key);
         printf(": ");
         printToken(val);
         printf("\n");
#endif

         if (strncmp(key.data, "x0", 2) == 0)
         {
            x0 = atof(val.data);
         }
         if (strncmp(key.data, "y0", 2) == 0)
         {
            y0 = atof(val.data);
         }
         if (strncmp(key.data, "x1", 2) == 0)
         {
            x1 = atof(val.data);
         }
         if (strncmp(key.data, "y1", 2) == 0)
         {
            y1 = atof(val.data);

            sum += ReferenceHaversine(x0, y0, x1, y1, 6372.8);
            num_entries += 1;
         }
      }
   }

   puts("jsonfile");
   printf("# of entries: %d\n", num_entries);
   double avg = sum / num_entries;
   printf("avg: %f\n", avg);
   return avg;
}

double parse_answersfile(FILE* f)
{
   TimeFunction;

   double sum = 0;
   int num_entries = 0;
   double entry[4];

   while (fread(entry, 4 * sizeof(double), 1, f))
   {
      double x0 = entry[0];
      double y0 = entry[1];
      double x1 = entry[2];
      double y1 = entry[3];
      sum += ReferenceHaversine(x0, y0, x1, y1, 6372.8);
      num_entries += 1;
   }

   puts("answersfile");
   printf("# of entries: %d\n", num_entries);
   double avg = sum / num_entries;
   printf("avg: %f\n", avg);
   return avg;
}

int main(int argc, char** argv)
{
   if (argc != 2 && argc != 3)
   {
      fprintf(stderr, "usage: $ ./parser jsonfile\n");
      fprintf(stderr, "usage: $ ./parser jsonfile answers.f64\n");
      return 1;
   }

   FILE* jsonfile = fopen(argv[1], "r");
   assert(jsonfile);
   double avg_json = parse_jsonfile(jsonfile);
   fclose(jsonfile);

   FILE* answersfile = NULL;
   if (argc == 3)
   {
      answersfile = fopen(argv[2], "r");
      assert(answersfile);
      double avg_answer = parse_answersfile(answersfile);
      fclose(answersfile);

      printf("error = %f\n", avg_answer - avg_json);
   }

   print_measurements();
   return 0;
}
