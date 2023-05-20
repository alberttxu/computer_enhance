#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include "utils.c"

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

struct MOVRegReg
{
   uint8_t opcode_d_w;
   uint8_t mod_reg_rm;
};

void decode_MOVRegReg(struct MOVRegReg *mov)
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

int decode_MOV(uint8_t *instruction)
{
   int instr_size = 0;
   if (*instruction >> 2 == 0x22)
   {
      instr_size = 2;
      decode_MOVRegReg((struct MOVRegReg *)instruction);
   }
   else
      assert(0);
   return instr_size;
}

void unit_tests(void)
{
   assert(sizeof(struct MOVRegReg) == 2);
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

   const int BUFSIZE = 1000;
   uint8_t *filedata = malloc(BUFSIZE);
   int filesize = readentirefile(f, filedata, BUFSIZE);

   puts("bits 16");
   int i = 0;
   while (i < filesize)
   {
      int instr_size = decode_MOV(&filedata[i]);
      assert(instr_size);
      i += instr_size;
   }

   free(filedata);
   return 0;
}
