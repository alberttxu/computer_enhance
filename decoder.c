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

void printMem(uint8_t rm, int disp)
{
   printf("[");
   print(EffectiveAddressTable[rm]);
   if (disp > 0)
   {
      printf("+%d", disp);
   }
   else if (disp < 0)
   {
      printf("%d", disp);
   }
   printf("]");
}

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
      disp = *(int8_t *)&mov->displo;
   else if (mod == 2)
      disp = *(int16_t *)&mov->displo;

   printf("mov ");
   if (d)
   {
      print(RegisterNames[reg][w]);
      printf(", ");
      if (mod == 0 && rm == 6)
      {
         instr_size = 4;
         int directaddress = *(int16_t *)&mov->displo;
         printf("[%d]", directaddress);
      }
      else
      {
         printMem(rm, disp);
      }
   }
   else
   {
      printMem(rm, disp);
      printf(", ");
      print(RegisterNames[reg][w]);
   }
   printf("\n");
   return instr_size;
}

struct MOVImmReg
{
   uint8_t opcode_w_reg;
   uint8_t datalo;
   uint8_t datahi;
};

int decode_MOVImmReg(struct MOVImmReg *mov)
{
   uint8_t w = (mov->opcode_w_reg >> 3) & 1;
   uint8_t reg = mov->opcode_w_reg & 0x7;
   printf("mov ");
   print(RegisterNames[reg][w]);
   printf(", ");
   printf("%d", mov->datalo + w * (mov->datahi << 8));
   printf("\n");

   int instr_size = w ? 3 : 2;
   return instr_size;
}

struct MOVImmRegMem
{
   uint8_t opcode_w;
   uint8_t mod_000_rm;
   uint8_t displo;
   uint8_t disphi;
   uint8_t datalo;
   uint8_t datahi;
};

int decode_MOVImmRegMem(struct MOVImmRegMem *mov)
{
   int instr_size = 0;
   assert((mov->opcode_w >> 1) == 0x63);
   assert(((mov->mod_000_rm >> 3) & 7) == 0);
   uint8_t w = mov->opcode_w & 1;
   uint8_t mod = (mov->mod_000_rm >> 6) & 3;
   uint8_t rm = mov->mod_000_rm & 7;

   if (mod == 0)
   {
      printf("mov ");
      printMem(rm, 0);
      printf(", ");
      int imm;
      if (w)
      {
      }
      else
      {
         instr_size = 3;
         imm = mov->displo;
         printf("byte %d", imm);
      }
      printf("\n");
   }
   else if (mod == 2)
   {
      printf("mov ");
      int disp = *(int16_t *)&mov->displo;
      printMem(rm, disp);
      printf(", ");
      int imm;
      if (w)
      {
         instr_size = 6;
         imm = mov->datalo + 256 * mov->datahi;
         printf("word %d", imm);
      }
      else
      {
      }
      printf("\n");
   }
   return instr_size;
}

struct MOVMemAccum
{
   uint8_t opcode_w;
   uint8_t addrlo;
   uint8_t addrhi;
};

void decode_MOVMemAccum(struct MOVMemAccum *mov)
{
   uint8_t w = mov->opcode_w & 1;
   printf("mov ");
   int addr;
   if (w)
   {
      addr = *(int16_t *)&mov->addrlo;
      printf("ax");
   }
   else
   {
      addr = mov->addrlo;
      printf("al");
   }
   printf(", ");
   printf("[%d]", addr);
   printf("\n");
}

void decode_MOVAccumMem(struct MOVMemAccum *mov)
{
   uint8_t w = mov->opcode_w & 1;
   printf("mov ");
   int addr;
   if (w)
   {
      addr = *(int16_t *)&mov->addrlo;
      printf("[%d]", addr);
      printf(", ");
      printf("ax");
   }
   else
   {
      addr = mov->addrlo;
      printf("[%d]", addr);
      printf(", ");
      printf("al");
   }
   printf("\n");
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
   else if (*instruction >> 1 == 0x63)
   {
      instr_size = decode_MOVImmRegMem((struct MOVImmRegMem *)instruction);
   }
   else if (*instruction >> 1 << 1 == 0xa0)
   {
      instr_size = 3;
      decode_MOVMemAccum((struct MOVMemAccum *)instruction);
   }
   else if (*instruction >> 1 << 1 == 0xa2)
   {
      instr_size = 3;
      decode_MOVAccumMem((struct MOVMemAccum *)instruction);
   }
   else
   {
      fprintf(stderr, "unimplemented MOV; first byte = 0x%x\n", *instruction);
      exit(1);
   }
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
