#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>

// taken from https://github.com/varnishcache/varnish-cache/blob/master/include/vas.h
#define AZ(foo)		do { assert((foo) == 0); } while (0)
#define AN(foo)		do { assert((foo) != 0); } while (0)

// debug macros
#define str(x) #x
#define showint(x) printf(str(x)" = %d\n", x)
#define showuint64(x) printf(str(x)" = %llu\n", x)
#define showfloat(x) printf(str(x)" = %f\n", (double)x)
#define showhex(x) printf(str(x)" = 0x%x\n", x)
#define showptr(x) printf(str(x)" = %p\n", x)
#define showaddr(x) printf("&"str(x)" = %p\n", &x)

typedef struct {
   const char *data;
   int length;
} String;

void print(String s)
{
   for (int i = 0; i < s.length; i++)
      printf("%c", s.data[i]);
}

void println(String s)
{
   print(s);
   printf("\n");
}

bool StringEQ(String a, String b)
{
   if (a.length != b.length)
      return false;

   for (int i = 0; i < a.length; i++)
   {
      if (a.data[i] != b.data[i])
         return false;
   }
   return true;
}

int readentirefile(FILE *f, uint8_t *buf, const int BUFSIZE)
{
   int filesize = 0;
   while(filesize < BUFSIZE && fread(&buf[filesize], 1, 1, f))
      filesize += 1;
   return filesize;
}

void raiseUnimplementedFunc(const char *file, int line)
{
   fprintf(stderr, "ERROR: unimplemented; %s line %d\n", file, line);
   exit(1);
}

#define raiseUnimplemented() raiseUnimplementedFunc(__FILE__, __LINE__)
