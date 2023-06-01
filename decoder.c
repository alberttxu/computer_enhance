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

struct MOVRegReg
{
   uint8_t opcode_d_w;
   uint8_t mod_reg_rm;
};

struct ADDRegReg
{
   uint8_t opcode_d_w;
   uint8_t mod_reg_rm;
};

struct SUBRegReg
{
   uint8_t opcode_d_w;
   uint8_t mod_reg_rm;
};

struct CMPRegReg
{
   uint8_t opcode_d_w;
   uint8_t mod_reg_rm;
};

int decode_MOVRegReg(struct MOVRegReg *mov)
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

   int instr_size = 2;
   return instr_size;
}

int decode_ADDRegReg(struct ADDRegReg *add)
{
   uint8_t opcode = add->opcode_d_w >> 2;
   assert(opcode == 0);
   uint8_t mod = add->mod_reg_rm >> 6;
   assert(mod == 3);
   uint8_t d = (add->opcode_d_w >> 1) & 1;
   uint8_t w = add->opcode_d_w & 1;
   uint8_t reg = (add->mod_reg_rm >> 3) & 0x7;
   uint8_t rm = add->mod_reg_rm & 0x7;

   int src = d ? rm : reg;
   int dst = d ? reg : rm;
   printf("add ");
   print(RegisterNames[dst][w]);
   printf(", ");
   print(RegisterNames[src][w]);
   printf("\n");

   int instr_size = 2;
   return instr_size;
}

int decode_SUBRegReg(struct SUBRegReg *sub)
{
   uint8_t opcode = sub->opcode_d_w >> 2;
   assert(opcode == 0xA);
   uint8_t mod = sub->mod_reg_rm >> 6;
   assert(mod == 3);
   uint8_t d = (sub->opcode_d_w >> 1) & 1;
   uint8_t w = sub->opcode_d_w & 1;
   uint8_t reg = (sub->mod_reg_rm >> 3) & 0x7;
   uint8_t rm = sub->mod_reg_rm & 0x7;

   int src = d ? rm : reg;
   int dst = d ? reg : rm;
   printf("sub ");
   print(RegisterNames[dst][w]);
   printf(", ");
   print(RegisterNames[src][w]);
   printf("\n");

   int instr_size = 2;
   return instr_size;
}

int decode_CMPRegReg(struct CMPRegReg *cmp)
{
   uint8_t opcode = cmp->opcode_d_w >> 2;
   assert(opcode == 14);
   uint8_t mod = cmp->mod_reg_rm >> 6;
   assert(mod == 3);
   uint8_t d = (cmp->opcode_d_w >> 1) & 1;
   uint8_t w = cmp->opcode_d_w & 1;
   uint8_t reg = (cmp->mod_reg_rm >> 3) & 0x7;
   uint8_t rm = cmp->mod_reg_rm & 0x7;

   int src = d ? rm : reg;
   int dst = d ? reg : rm;
   printf("cmp ");
   print(RegisterNames[dst][w]);
   printf(", ");
   print(RegisterNames[src][w]);
   printf("\n");

   int instr_size = 2;
   return instr_size;
}

struct MOVRegMem
{
   uint8_t opcode_d_w;
   uint8_t mod_reg_rm;
   uint8_t displo;
   uint8_t disphi;
};

struct ADDRegMem
{
   uint8_t opcode_d_w;
   uint8_t mod_reg_rm;
   uint8_t displo;
   uint8_t disphi;
};

struct SUBRegMem
{
   uint8_t opcode_d_w;
   uint8_t mod_reg_rm;
   uint8_t displo;
   uint8_t disphi;
};

struct CMPRegMem
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

int decode_ADDRegMem(struct ADDRegMem *add)
{
   int instr_size = 0;
   uint8_t opcode = add->opcode_d_w >> 2;
   assert(opcode == 0);
   uint8_t mod = add->mod_reg_rm >> 6;
   instr_size = 2 + mod;
   uint8_t d = (add->opcode_d_w >> 1) & 1;
   uint8_t w = add->opcode_d_w & 1;
   uint8_t reg = (add->mod_reg_rm >> 3) & 0x7;
   uint8_t rm = add->mod_reg_rm & 0x7;
   int disp = 0;
   if (mod == 1)
      disp = *(int8_t *)&add->displo;
   else if (mod == 2)
      disp = *(int16_t *)&add->displo;

   printf("add ");
   if (d)
   {
      print(RegisterNames[reg][w]);
      printf(", ");
      if (mod == 0 && rm == 6)
      {
         instr_size = 4;
         int directaddress = *(int16_t *)&add->displo;
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

int decode_SUBRegMem(struct SUBRegMem *sub)
{
   int instr_size = 0;
   uint8_t opcode = sub->opcode_d_w >> 2;
   assert(opcode == 0xA);
   uint8_t mod = sub->mod_reg_rm >> 6;
   instr_size = 2 + mod;
   uint8_t d = (sub->opcode_d_w >> 1) & 1;
   uint8_t w = sub->opcode_d_w & 1;
   uint8_t reg = (sub->mod_reg_rm >> 3) & 0x7;
   uint8_t rm = sub->mod_reg_rm & 0x7;
   int disp = 0;
   if (mod == 1)
      disp = *(int8_t *)&sub->displo;
   else if (mod == 2)
      disp = *(int16_t *)&sub->displo;

   printf("sub ");
   if (d)
   {
      print(RegisterNames[reg][w]);
      printf(", ");
      if (mod == 0 && rm == 6)
      {
         instr_size = 4;
         int directaddress = *(int16_t *)&sub->displo;
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

int decode_CMPRegMem(struct CMPRegMem *cmp)
{
   int instr_size = 0;
   uint8_t opcode = cmp->opcode_d_w >> 2;
   assert(opcode == 14);
   uint8_t mod = cmp->mod_reg_rm >> 6;
   instr_size = 2 + mod;
   uint8_t d = (cmp->opcode_d_w >> 1) & 1;
   uint8_t w = cmp->opcode_d_w & 1;
   uint8_t reg = (cmp->mod_reg_rm >> 3) & 0x7;
   uint8_t rm = cmp->mod_reg_rm & 0x7;
   int disp = 0;
   if (mod == 1)
      disp = *(int8_t *)&cmp->displo;
   else if (mod == 2)
      disp = *(int16_t *)&cmp->displo;

   printf("cmp ");
   if (d)
   {
      print(RegisterNames[reg][w]);
      printf(", ");
      if (mod == 0 && rm == 6)
      {
         instr_size = 4;
         int directaddress = *(int16_t *)&cmp->displo;
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

struct ADDImmRegMem
{
   uint8_t opcode_sw;
   uint8_t mod_000_rm;
   uint8_t displo;
   uint8_t disphi;
   uint8_t datalo;
   uint8_t datahi;
};

struct SUBImmRegMem
{
   uint8_t opcode_sw;
   uint8_t mod_101_rm;
   uint8_t displo;
   uint8_t disphi;
   uint8_t datalo;
   uint8_t datahi;
};

struct CMPImmRegMem
{
   uint8_t opcode_sw;
   uint8_t mod_111_rm;
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

int decode_ADDImmRegMem(struct ADDImmRegMem *add)
{
   int instr_size = 0;
   assert((add->opcode_sw >> 2) == 32);
   assert(((add->mod_000_rm >> 3) & 7) == 0);
   uint8_t w = add->opcode_sw & 1;
   uint8_t s = (add->opcode_sw >> 1) & 1;
   uint8_t mod = (add->mod_000_rm >> 6) & 3;
   uint8_t rm = add->mod_000_rm & 7;
   printf("add ");

   if (mod == 0)
   {
      int imm;
      if (s)
      {
         if (w)
         {
            instr_size = 3;
            imm = *(int8_t *)&add->displo;
            printf("word ");
         }
         else
            assert(0);
      }
      else
      {
         if (w)
         {
            instr_size = 4;
            imm = *(uint16_t *)&add->displo;
            printf("word ");
         }
         else
         {
            instr_size = 3;
            imm = *(uint8_t *)&add->displo;
            printf("byte ");
         }
      }
      printMem(rm, 0);
      printf(", %d", imm);
      printf("\n");
   }

   else if (mod == 1)
   {
   }

   else if (mod == 2)
   {
      int disp = *(int16_t *)&add->displo;
      int imm;
      if (s)
      {
         if (w)
         {
            instr_size = 5;
            imm = *(int8_t *)&add->datalo;
            printf("word ");
         }
         else
            assert(0);
      }
      else
      {
         if (w)
         {
            instr_size = 6;
            imm = *(uint16_t *)&add->datalo;
            printf("word ");
         }
         else
         {
            instr_size = 5;
            imm = *(uint8_t *)&add->datalo;
            printf("byte ");
         }
      }
      printMem(rm, disp);
      printf(", %d", imm);
      printf("\n");
   }

   else if (mod == 3)
   {
      print(RegisterNames[rm][w]);
      int imm;
      if (s)
      {
         if (w)
         {
            instr_size = 3;
            imm = *(int8_t *)&add->displo;
         }
         else
            assert(0);
      }
      else
      {
         if (w)
         {
            instr_size = 4;
            imm = *(uint16_t *)&add->displo;
         }
         else
         {
            instr_size = 3;
            imm = *(uint8_t *)&add->displo;
         }
      }
      printf(", %d\n", imm);
   }
   return instr_size;
}

int decode_SUBImmRegMem(struct SUBImmRegMem *sub)
{
   int instr_size = 0;
   assert((sub->opcode_sw >> 2) == 32);
   assert(((sub->mod_101_rm >> 3) & 7) == 5);
   uint8_t w = sub->opcode_sw & 1;
   uint8_t s = (sub->opcode_sw >> 1) & 1;
   uint8_t mod = (sub->mod_101_rm >> 6) & 3;
   uint8_t rm = sub->mod_101_rm & 7;
   printf("sub ");

   if (mod == 0)
   {
      int imm;
      if (s)
      {
         if (w)
         {
            instr_size = 3;
            imm = *(int8_t *)&sub->displo;
            printf("word ");
         }
         else
            assert(0);
      }
      else
      {
         if (w)
         {
            instr_size = 4;
            imm = *(uint16_t *)&sub->displo;
            printf("word ");
         }
         else
         {
            instr_size = 3;
            imm = *(uint8_t *)&sub->displo;
            printf("byte ");
         }
      }
      printMem(rm, 0);
      printf(", %d", imm);
      printf("\n");
   }

   else if (mod == 1)
   {
   }

   else if (mod == 2)
   {
      int disp = *(int16_t *)&sub->displo;
      int imm;
      if (s)
      {
         if (w)
         {
            instr_size = 5;
            imm = *(int8_t *)&sub->datalo;
            printf("word ");
         }
         else
            assert(0);
      }
      else
      {
         if (w)
         {
            instr_size = 6;
            imm = *(uint16_t *)&sub->datalo;
            printf("word ");
         }
         else
         {
            instr_size = 5;
            imm = *(uint8_t *)&sub->datalo;
            printf("byte ");
         }
      }
      printMem(rm, disp);
      printf(", %d", imm);
      printf("\n");
   }

   else if (mod == 3)
   {
      print(RegisterNames[rm][w]);
      int imm;
      if (s)
      {
         if (w)
         {
            instr_size = 3;
            imm = *(int8_t *)&sub->displo;
         }
         else
            assert(0);
      }
      else
      {
         if (w)
         {
            instr_size = 4;
            imm = *(uint16_t *)&sub->displo;
         }
         else
         {
            instr_size = 3;
            imm = *(uint8_t *)&sub->displo;
         }
      }
      printf(", %d\n", imm);
   }
   return instr_size;
}

int decode_CMPImmRegMem(struct CMPImmRegMem *cmp)
{
   int instr_size = 0;
   assert((cmp->opcode_sw >> 2) == 32);
   assert(((cmp->mod_111_rm >> 3) & 7) == 7);
   uint8_t w = cmp->opcode_sw & 1;
   uint8_t s = (cmp->opcode_sw >> 1) & 1;
   uint8_t mod = (cmp->mod_111_rm >> 6) & 3;
   uint8_t rm = cmp->mod_111_rm & 7;
   printf("cmp ");

   if (mod == 0)
   {
      if (rm == 6)
      {
         instr_size = 6;
         int addr = *(uint16_t *)&cmp->displo;
         int imm;
         if (w)
         {
            if (s)
            {
               instr_size = 5;
               imm = *(int8_t *)&cmp->datalo;
               printf("word ");
            }
            else
            {
               instr_size = 6;
               imm = *(int16_t *)&cmp->datalo;
               printf("word ");
            }
         }
         else
         {
            instr_size = 5;
            imm = *(int8_t *)&cmp->datalo;
            printf("byte ");
         }
         printf("[%d], %d\n", addr, imm);
      }

      else
      {
         int imm;
         if (s)
         {
            if (w)
            {
               instr_size = 3;
               imm = *(int8_t *)&cmp->displo;
               printf("word ");
            }
            else
               assert(0);
         }
         else
         {
            if (w)
            {
               instr_size = 4;
               imm = *(uint16_t *)&cmp->displo;
               printf("word ");
            }
            else
            {
               instr_size = 3;
               imm = *(uint8_t *)&cmp->displo;
               printf("byte ");
            }
         }
         printMem(rm, 0);
         printf(", %d", imm);
         printf("\n");
      }
   }

   else if (mod == 1)
   {
   }

   else if (mod == 2)
   {
      int disp = *(int16_t *)&cmp->displo;
      int imm;
      if (s)
      {
         if (w)
         {
            instr_size = 5;
            imm = *(int8_t *)&cmp->datalo;
            printf("word ");
         }
         else
            assert(0);
      }
      else
      {
         if (w)
         {
            instr_size = 6;
            imm = *(uint16_t *)&cmp->datalo;
            printf("word ");
         }
         else
         {
            instr_size = 5;
            imm = *(uint8_t *)&cmp->datalo;
            printf("byte ");
         }
      }
      printMem(rm, disp);
      printf(", %d", imm);
      printf("\n");
   }

   else if (mod == 3)
   {
      print(RegisterNames[rm][w]);
      int imm;
      if (s)
      {
         if (w)
         {
            instr_size = 3;
            imm = *(int8_t *)&cmp->displo;
         }
         else
            assert(0);
      }
      else
      {
         if (w)
         {
            instr_size = 4;
            imm = *(uint16_t *)&cmp->displo;
         }
         else
         {
            instr_size = 3;
            imm = *(uint8_t *)&cmp->displo;
         }
      }
      printf(", %d\n", imm);
   }
   return instr_size;
}

struct MOVMemAccum
{
   uint8_t opcode_w;
   uint8_t addrlo;
   uint8_t addrhi;
};

int decode_MOVMemAccum(struct MOVMemAccum *mov)
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

   int instr_size = 3;
   return instr_size;
}

int decode_MOVAccumMem(struct MOVMemAccum *mov)
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

   int instr_size = 3;
   return instr_size;
}

struct ADDImmAccum
{
   uint8_t opcode_w;
   uint8_t addrlo;
   uint8_t addrhi;
};

struct SUBImmAccum
{
   uint8_t opcode_w;
   uint8_t addrlo;
   uint8_t addrhi;
};

struct CMPImmAccum
{
   uint8_t opcode_w;
   uint8_t addrlo;
   uint8_t addrhi;
};

int decode_ADDImmAccum(struct ADDImmAccum *add)
{
   int instr_size = 0;
   uint8_t w = add->opcode_w & 1;
   printf("add ");
   int imm;
   if (w)
   {
      instr_size = 3;
      imm = *(int16_t *)&add->addrlo;
      printf("ax");
   }
   else
   {
      instr_size = 2;
      imm = *(int8_t *)&add->addrlo;
      printf("al");
   }
   printf(", %d\n", imm);
   return instr_size;
}

int decode_SUBImmAccum(struct SUBImmAccum *sub)
{
   int instr_size = 0;
   uint8_t w = sub->opcode_w & 1;
   printf("sub ");
   int imm;
   if (w)
   {
      instr_size = 3;
      imm = *(int16_t *)&sub->addrlo;
      printf("ax");
   }
   else
   {
      instr_size = 2;
      imm = *(int8_t *)&sub->addrlo;
      printf("al");
   }
   printf(", %d\n", imm);
   return instr_size;
}


int decode_CMPImmAccum(struct CMPImmAccum *cmp)
{
   int instr_size = 0;
   uint8_t w = cmp->opcode_w & 1;
   printf("cmp ");
   int imm;
   if (w)
   {
      instr_size = 3;
      imm = *(int16_t *)&cmp->addrlo;
      printf("ax");
   }
   else
   {
      instr_size = 2;
      imm = *(int8_t *)&cmp->addrlo;
      printf("al");
   }
   printf(", %d\n", imm);
   return instr_size;
}

int decode_instruction(uint8_t *instruction)
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
            instr_size = decode_MOVRegReg((struct MOVRegReg *)instruction);
            break;
         default:
            assert(0);
      }
   }

   else if (*instruction >> 2 == 0)
   {
      uint8_t mod = (instruction[1] >> 6) & 3;
      switch (mod)
      {
         case 0:
         case 1:
         case 2:
            instr_size = decode_ADDRegMem((struct ADDRegMem *)instruction);
            break;
         case 3:
            instr_size = decode_ADDRegReg((struct ADDRegReg *)instruction);
            break;
         default:
            assert(0);
      }
   }

   else if (*instruction >> 2 == 0xA)
   {
      uint8_t mod = (instruction[1] >> 6) & 3;
      switch (mod)
      {
         case 0:
         case 1:
         case 2:
            instr_size = decode_SUBRegMem((struct SUBRegMem *)instruction);
            break;
         case 3:
            instr_size = decode_SUBRegReg((struct SUBRegReg *)instruction);
            break;
         default:
            assert(0);
      }
   }

   else if (*instruction >> 2 == 14)
   {
      uint8_t mod = (instruction[1] >> 6) & 3;
      switch (mod)
      {
         case 0:
         case 1:
         case 2:
            instr_size = decode_CMPRegMem((struct CMPRegMem *)instruction);
            break;
         case 3:
            instr_size = decode_CMPRegReg((struct CMPRegReg *)instruction);
            break;
         default:
            assert(0);
      }
   }

   else if (*instruction >> 4 == 0xB)
      instr_size = decode_MOVImmReg((struct MOVImmReg *)instruction);

   else if (*instruction >> 1 == 0x63)
      instr_size = decode_MOVImmRegMem((struct MOVImmRegMem *)instruction);

   else if (*instruction >> 2 == 32)
   {
      uint8_t micro_op = (instruction[1] >> 3) & 7;
      if (micro_op == 0)
         instr_size = decode_ADDImmRegMem((struct ADDImmRegMem *)instruction);
      else if (micro_op == 5)
         instr_size = decode_SUBImmRegMem((struct SUBImmRegMem *)instruction);
      else if (micro_op == 7)
         instr_size = decode_CMPImmRegMem((struct CMPImmRegMem *)instruction);
      else
      {
         fprintf(stderr, "unimplemented micro_op %d\n", micro_op);
         exit(1);
      }
   }

   else if (*instruction >> 1 << 1 == 0xa0)
      instr_size = decode_MOVMemAccum((struct MOVMemAccum *)instruction);

   else if (*instruction >> 1 << 1 == 0xa2)
      instr_size = decode_MOVAccumMem((struct MOVMemAccum *)instruction);

   else if (*instruction >> 1 == 2)
      instr_size = decode_ADDImmAccum((struct ADDImmAccum *)instruction);

   else if (*instruction >> 1 == 22)
      instr_size = decode_SUBImmAccum((struct SUBImmAccum *)instruction);

   else if (*instruction >> 1 == 30)
      instr_size = decode_CMPImmAccum((struct CMPImmAccum *)instruction);

   else
   {
      fprintf(stderr, "unimplemented opcode; first byte = 0x%x\n", *instruction);
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
      int instr_size = decode_instruction(&filedata[i]);
      assert(instr_size);
      i += instr_size;
   }

   free(filedata);
   return 0;
}
