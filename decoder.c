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

String EffectiveAddressTable[8] = {
   {"bx+si", 5},
   {"bx+di", 5},
   {"bp+si", 5},
   {"bp+di", 5},
   {"si", 2},
   {"di", 2},
   {"bp", 2},
   {"bx", 2},
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

struct MOVRegMem
{
   uint8_t opcode_d_w;
   uint8_t mod_reg_rm;
   uint8_t displo;
   uint8_t disphi;
};

int decode_MOVRegMem(struct MOVRegMem *mov)
{
   int instr_size = 0;
   uint8_t opcode = mov->opcode_d_w >> 2;
   assert(opcode == 0x22);
   uint8_t mod = mov->mod_reg_rm >> 6;
   instr_size = 2 + mod;
   uint8_t d = (mov->opcode_d_w >> 1) & 1;
   uint8_t w = mov->opcode_d_w & 1;
   uint8_t reg = (mov->mod_reg_rm >> 3) & 0x7;
   uint8_t rm = mov->mod_reg_rm & 0x7;
   int disp = 0;
   if (mod == 1)
      disp = mov->displo;
   else if (mod == 2)
      disp = mov->displo + 256 * mov->disphi;

   printf("mov ");
   if (d)
   {
      print(RegisterNames[reg][w]);
      printf(", ");
      printf("[");
      print(EffectiveAddressTable[rm]);
      if (disp)
      {
         printf("+%d", disp);
      }
      printf("]");
   }
   else
   {
      printf("[");
      print(EffectiveAddressTable[rm]);
      if (disp)
      {
         printf("+%d", disp);
      }
      printf("]");
      printf(", ");
      print(RegisterNames[reg][w]);
   }
   printf("\n");
   return instr_size;
}

struct MOVImmReg
{
   uint8_t opcode_w_reg;
   uint8_t data;
   uint8_t dataifwide;
};

int decode_MOVImmReg(struct MOVImmReg *mov)
{
   uint8_t w = (mov->opcode_w_reg >> 3) & 1;
   uint8_t reg = mov->opcode_w_reg & 0x7;
   printf("mov ");
   print(RegisterNames[reg][w]);
   printf(", ");
   printf("%d", mov->data + w * (mov->dataifwide << 8));
   printf("\n");

   int instr_size = w ? 3 : 2;
   return instr_size;
}

int decode_MOV(uint8_t *instruction)
{
   int instr_size = 0;
   if (*instruction >> 2 == 0x22)
   {
      uint8_t mod = (instruction[1] >> 6) & 3;
      switch (mod)
      {
         case 0:
         case 1:
         case 2:
            instr_size = decode_MOVRegMem((struct MOVRegMem *)instruction);
            break;
         case 3:
            instr_size = 2;
            decode_MOVRegReg((struct MOVRegReg *)instruction);
            break;
         default:
            assert(0);
      }
   }
   else if (*instruction >> 4 == 0xB)
   {
      instr_size = decode_MOVImmReg((struct MOVImmReg *)instruction);
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
