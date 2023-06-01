#include <stdio.h>
#include <stdint.h>

// debug macros
#define str(x) #x
#define showint(x) printf(str(x)" = %d\n", x)
#define showuint64(x) printf(str(x)" = %llu\n", x)
#define showfloat(x) printf(str(x)" = %f\n", (double)x)
#define showhex(x) printf(str(x)" = 0x%x\n", x)
#define showptr(x) printf(str(x)" = %p\n", x)
#define showaddr(x) printf("&" str(x)" = %p\n", &x)

typedef struct {
   char *data;
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

int readentirefile(FILE *f, uint8_t *buf, const int BUFSIZE)
{
   int filesize = 0;
   while(filesize < BUFSIZE && fread(&buf[filesize], 1, 1, f))
      filesize += 1;
   return filesize;
}
