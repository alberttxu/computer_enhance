#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

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

String RegisterNames[8][2] = {
   {{"al", 2}, {"ax", 2}},
   {{"cl", 2}, {"cx", 2}},
   {{"dl", 2}, {"dx", 2}},
   {{"bl", 2}, {"bx", 2}},
   {{"ah", 2}, {"sp", 2}},
   {{"ch", 2}, {"bp", 2}},
   {{"dh", 2}, {"si", 2}},
   {{"bh", 2}, {"di", 2}},
};

struct MOV
{
   uint8_t opcode_d_w;
   uint8_t mod_reg_rm;
};

void decode_MOVRegReg(struct MOV *mov)
{
   uint8_t opcode = mov->opcode_d_w >> 2;
   assert(opcode == 0x22);
   uint8_t mod = mov->mod_reg_rm >> 6;
   assert(mod == 3);
   uint8_t d = (mov->opcode_d_w >> 1) & 1;
   uint8_t w = mov->opcode_d_w & 1;
   uint8_t reg = (mov->mod_reg_rm >> 3) & 0x7;
   uint8_t rm = mov->mod_reg_rm & 0x7;

   int src = d ? rm : reg;
   int dst = d ? reg : rm;
   printf("mov ");
   print(RegisterNames[dst][w]);
   printf(", ");
   print(RegisterNames[src][w]);
   printf("\n");
}

void unit_tests(void)
{
   assert(sizeof(struct MOV) == 2);
}

int main(int argc, char **argv)
{
   unit_tests();
   if (argc != 2)
   {
      puts("need objectfile for the 1st argument");
      exit(1);
   }
   char *objectfile = argv[1];
   FILE *f = fopen(objectfile, "r");
   assert(f);

   puts("bits 16");
   struct MOV mov;
   while(fread(&mov, sizeof(struct MOV), 1, f))
   {
      decode_MOVRegReg(&mov);
   }
   return 0;
}
