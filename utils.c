#include <stdio.h>

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
