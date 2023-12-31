#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include "utils.c"

bool simulate = false;
bool showclockcycles = false;
int totalcycles = 0;

struct CPU_Registers
{
   uint16_t ax;
   uint16_t bx;
   uint16_t cx;
   uint16_t dx;
   uint16_t sp;
   uint16_t bp;
   uint16_t si;
   uint16_t di;
   // segment registers
   uint16_t es;
   uint16_t cs;
   uint16_t ss;
   uint16_t ds;
   // other
   int ip;
} registers;

struct CPU_FLAGS
{
   bool ZF;
   bool SF;
} flags;

#define MEMORYSIZE 1 << 16

uint8_t memory[MEMORYSIZE];

void dumpmemory(FILE *f)
{
   int n = fwrite(memory, 1, MEMORYSIZE, f);
   assert(n == MEMORYSIZE);
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

String EffectiveAddressRegNames[8] = {
   {"bx+si", 5},
   {"bx+di", 5},
   {"bp+si", 5},
   {"bp+di", 5},
   {"si", 2},
   {"di", 2},
   {"bp", 2},
   {"bx", 2},
};

String SegmentRegisterNames[4] = {
   {"es", 2},
   {"cs", 2},
   {"ss", 2},
   {"ds", 2}
};

void setFlags(int result, int w)
{
   flags.ZF = result == 0;
   if (w)
      flags.SF = result & 0x8000;
   else
      flags.SF = result & 0x80;
}

void printFlags(void)
{
   printf("(");
   if (flags.ZF)
      printf("Z");
   if (flags.SF)
      printf("S");
   printf(")");
}

void printRegisters(void)
{
   printf("; ax: 0x%04x\n", registers.ax);
   printf("; bx: 0x%04x\n", registers.bx);
   printf("; cx: 0x%04x\n", registers.cx);
   printf("; dx: 0x%04x\n", registers.dx);
   printf("; sp: 0x%04x\n", registers.sp);
   printf("; bp: 0x%04x\n", registers.bp);
   printf("; si: 0x%04x\n", registers.si);
   printf("; di: 0x%04x\n", registers.di);
   printf("; es: 0x%04x\n", registers.es);
   printf("; cs: 0x%04x\n", registers.cs);
   printf("; ss: 0x%04x\n", registers.ss);
   printf("; ds: 0x%04x\n", registers.ds);
   printf("; ip: 0x%04x\n", registers.ip);
}

int getReg(int reg, int w)
{
   int val = 0;
   if (w)
   {
      if (reg == 0)
         val = registers.ax;
      else if (reg == 1)
         val = registers.cx;
      else if (reg == 2)
         val = registers.dx;
      else if (reg == 3)
         val = registers.bx;
      else if (reg == 4)
         val = registers.sp;
      else if (reg == 5)
         val = registers.bp;
      else if (reg == 6)
         val = registers.si;
      else if (reg == 7)
         val = registers.di;
      else
         assert(0);
   }
   else
   {
      if (reg == 0)
         val = registers.ax & 0xff;
      else if (reg == 1)
         val = registers.cx & 0xff;
      else if (reg == 2)
         val = registers.dx & 0xff;
      else if (reg == 3)
         val = registers.bx & 0xff;
      else if (reg == 4)
         val = registers.ax >> 8;
      else if (reg == 5)
         val = registers.cx >> 8;
      else if (reg == 6)
         val = registers.dx >> 8;
      else if (reg == 7)
         val = registers.bx >> 8;
      else
         assert(0);
   }
   return val;
}

void setReg(int reg, int w, int val)
{
   if (w)
   {
      if (reg == 0)
         registers.ax = val;
      else if (reg == 1)
         registers.cx = val;
      else if (reg == 2)
         registers.dx = val;
      else if (reg == 3)
         registers.bx = val;
      else if (reg == 4)
         registers.sp = val;
      else if (reg == 5)
         registers.bp = val;
      else if (reg == 6)
         registers.si = val;
      else if (reg == 7)
         registers.di = val;
      else
         assert(0);
   }
   else
   {
      if (reg == 0)
         *(uint8_t *)&registers.ax = val;
      else if (reg == 1)
         *(uint8_t *)&registers.cx = val;
      else if (reg == 2)
         *(uint8_t *)&registers.dx = val;
      else if (reg == 3)
         *(uint8_t *)&registers.bx = val;
      else if (reg == 4)
         *(1 + (uint8_t *)&registers.ax) = val;
      else if (reg == 5)
         *(1 + (uint8_t *)&registers.cx) = val;
      else if (reg == 6)
         *(1 + (uint8_t *)&registers.dx) = val;
      else if (reg == 7)
         *(1 + (uint8_t *)&registers.bx) = val;
      else
         assert(0);
   }
}

uint16_t getSegmentReg(int SR)
{
   uint16_t val = 0;
   if (SR == 0)
      val = registers.es;
   else if (SR == 1)
      val = registers.cs;
   else if (SR == 2)
      val = registers.ss;
   else if (SR == 3)
      val = registers.ds;
   else
      assert(0);
   return val;
}

void setSegmentReg(int SR, uint16_t val)
{
   if (SR == 0)
      registers.es = val;
   else if (SR == 1)
      registers.cs = val;
   else if (SR == 2)
      registers.ss = val;
   else if (SR == 3)
      registers.ds = val;
   else
      assert(0);
}

void printMem(uint8_t rm, int disp)
{
   printf("[");
   print(EffectiveAddressRegNames[rm]);
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

int getEffectiveAddress(int rm, int disp)
{
   int addr;
   if (rm == 0)
      addr = registers.bx + registers.si;
   else if (rm == 1)
      addr = registers.bx + registers.di;
   else if (rm == 2)
      addr = registers.bp + registers.si;
   else if (rm == 3)
      addr = registers.bp + registers.di;
   else if (rm == 4)
      addr = registers.si;
   else if (rm == 5)
      addr = registers.di;
   else if (rm == 6)
      addr = registers.bp;
   else if (rm == 7)
      addr = registers.bx;
   else
      assert(0);

   addr += disp;
   return addr;
}

int getEffectiveAddressCycles(int rm, int disp)
{
   int clocks;
   if (disp == 0)
   {
      if (rm == 0)
         clocks = 7;
      else if (rm == 1)
         clocks = 8;
      else if (rm == 2)
         clocks = 8;
      else if (rm == 3)
         clocks = 7;
      else if (rm == 4)
         clocks = 5;
      else if (rm == 5)
         clocks = 5;
      else if (rm == 6)
         clocks = 5;
      else if (rm == 7)
         clocks = 5;
      else
         assert(0);
   }

   else
   {
      if (rm == 0)
         clocks = 11;
      else if (rm == 1)
         clocks = 12;
      else if (rm == 2)
         clocks = 12;
      else if (rm == 3)
         clocks = 11;
      else if (rm == 4)
         clocks = 9;
      else if (rm == 5)
         clocks = 9;
      else if (rm == 6)
         clocks = 9;
      else if (rm == 7)
         clocks = 9;
      else
         assert(0);
   }
   return clocks;
}

struct MOVRegReg
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

   if (simulate)
   {
      int old_value = getReg(dst, w);
      setReg(dst, w, getReg(src, w));
      int new_value = getReg(dst, w);
      printf("; 0x%x -> 0x%x", old_value, new_value);
   }

   if (showclockcycles)
   {
      int clocks = 2;
      totalcycles += clocks;
      printf("; %d clocks, total = %d", clocks, totalcycles);
   }

   printf("\n");

   int instr_size = 2;
   return instr_size;
}

struct ADDRegReg
{
   uint8_t opcode_d_w;
   uint8_t mod_reg_rm;
};

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

   if (simulate)
   {
      int old_value = getReg(dst, w);
      setReg(dst, w, old_value + getReg(src, w));
      int new_value = getReg(dst, w);
      flags.ZF = new_value == 0;
      flags.SF = new_value < 0;
      printf("; 0x%x -> 0x%x, flags = ", old_value, new_value);
      printFlags();
   }

   if (showclockcycles)
   {
      int clocks = 3;
      totalcycles += clocks;
      printf("; %d clocks, total = %d", clocks, totalcycles);
   }

   printf("\n");

   int instr_size = 2;
   return instr_size;
}

struct SUBRegReg
{
   uint8_t opcode_d_w;
   uint8_t mod_reg_rm;
};

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

   if (simulate)
   {
      int old_value = getReg(dst, w);
      setReg(dst, w, old_value - getReg(src, w));
      int new_value = getReg(dst, w);
      setFlags(new_value, w);
      printf("; %x -> %x, flags = ", old_value, new_value);
      printFlags();
   }

   printf("\n");

   int instr_size = 2;
   return instr_size;
}

struct CMPRegReg
{
   uint8_t opcode_d_w;
   uint8_t mod_reg_rm;
};

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

   if (simulate)
   {
      uint16_t result = getReg(dst, w) - getReg(src, w);
      setFlags(result, w);
      printf("; flags = ");
      printFlags();
   }

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

int decode_MOVRegMem(struct MOVRegMem *mov)
{
   int instr_size = 0;
   uint8_t opcode = mov->opcode_d_w >> 2;
   assert(opcode == 0x22);
   uint8_t mod = mov->mod_reg_rm >> 6;
   assert(mod < 3);
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

   if (d) // mem to reg
   {
      printf("mov ");
      print(RegisterNames[reg][w]);
      printf(", ");

      int addr;
      int memory_value;
      int clocks = 0;
      if (mod == 0 && rm == 6)
      {
         instr_size = 4;
         addr = *(int16_t *)&mov->displo;
         printf("[%d]", addr);

         if (simulate)
            memory_value = memory[addr];

         if (showclockcycles)
         {
            clocks = 8 + 6;
            totalcycles += clocks;
         }
      }
      else
      {
         printMem(rm, disp);

         if (simulate)
         {
            addr = getEffectiveAddress(rm, disp);
            memory_value = memory[addr];
         }

         if (showclockcycles)
         {
            clocks = 8 + getEffectiveAddressCycles(rm, disp);
            totalcycles += clocks;
         }
      }

      if (simulate)
      {
         int old_value = getReg(reg, w);
         setReg(reg, w, memory_value);
         int new_value = getReg(reg, w);
         if (w)
            printf("; 0x%04x -> 0x%04x = memory[%d]", old_value, new_value, addr);
         else
            printf("; 0x%02x -> 0x%02x = memory[%d]", old_value, new_value, addr);
      }

      if (showclockcycles)
         printf("; %d clocks, total = %d", clocks, totalcycles);
   }
   else // reg to mem
   {
      if (mod == 0 && rm == 6)
      {
         raiseUnimplemented();
      }
      else
      {
         printf("mov ");
         printMem(rm, disp);
         printf(", ");
         print(RegisterNames[reg][w]);

         if (simulate)
         {
            int addr = getEffectiveAddress(rm, disp);
            int old_value = memory[addr];
            memory[addr] = getReg(reg, w);
            int new_value = memory[addr];
            if (w)
               printf("; memory[%d]: [0x%04x] -> [0x%04x]", addr, old_value, new_value);
            else
               printf("; memory[%d]: [0x%02x] -> [0x%02x]", addr, old_value, new_value);
         }

         if (showclockcycles)
         {
            int clocks = 9 + getEffectiveAddressCycles(rm, disp);
            totalcycles += clocks;
            printf("; %d clocks, total = %d", clocks, totalcycles);
         }
      }
   }
   printf("\n");
   return instr_size;
}

struct ADDRegMem
{
   uint8_t opcode_d_w;
   uint8_t mod_reg_rm;
   uint8_t displo;
   uint8_t disphi;
};

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


   if (d) // mem to reg
   {
      printf("add ");
      print(RegisterNames[reg][w]);
      printf(", ");

      int addr;
      int memory_value;
      if (mod == 0 && rm == 6)
      {
         instr_size = 4;
         addr = *(int16_t *)&add->displo;
         printf("[%d]", addr);
         if (simulate)
            memory_value = memory[addr];
      }
      else
      {
         printMem(rm, disp);
         if (simulate)
         {
            addr = getEffectiveAddress(rm, disp);
            memory_value = memory[addr];
         }
      }

      if (simulate)
      {
         int old_value = getReg(reg, w);
         setReg(reg, w, old_value + memory_value);
         int new_value = getReg(reg, w);
         printf("; 0x%x -> 0x%x = memory[%d]", old_value, new_value, addr);
      }
   }
   else
   {
      printf("add ");
      printMem(rm, disp);
      printf(", ");
      print(RegisterNames[reg][w]);

      if (showclockcycles)
      {
         int clocks = 16 + getEffectiveAddressCycles(rm, disp);
         totalcycles += clocks;
         printf("; %d clocks, total = %d", clocks, totalcycles);
      }
   }
   printf("\n");
   return instr_size;
}

struct SUBRegMem
{
   uint8_t opcode_d_w;
   uint8_t mod_reg_rm;
   uint8_t displo;
   uint8_t disphi;
};

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

struct CMPRegMem
{
   uint8_t opcode_d_w;
   uint8_t mod_reg_rm;
   uint8_t displo;
   uint8_t disphi;
};

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
   int imm = mov->datalo + w * (mov->datahi << 8);
   printf("%d", imm);

   if (simulate)
   {
      int old_value = getReg(reg, w);
      setReg(reg, w, imm);
      int new_value = getReg(reg, w);
      printf("; 0x%x -> 0x%x", old_value, new_value);
   }

   if (showclockcycles)
   {
      int clocks = 4;
      totalcycles += clocks;
      printf("; %d clocks, total = %d", clocks, totalcycles);
   }

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
      int imm = 0;
      if (w)
      {
         printf("word ");
         if (rm == 6)
         {
            instr_size = 6;
            uint16_t disp = *(uint16_t *)&mov->displo;
            imm = *(int16_t *)&mov->datalo;
            printf("[%d]", disp);
            printf(", %d", imm);

            if (simulate)
            {
               int old_value = *(uint16_t *)&memory[disp];
               *(uint16_t *)&memory[disp] = imm;
               int new_value = *(uint16_t *)&memory[disp];
               printf("; 0x%04x -> 0x%04x", old_value, new_value);
            }
         }
         else
         {
            raiseUnimplemented();
         }
      }
      else
      {
         instr_size = 2 + (w + 1);
         imm = mov->displo + w * 256 * mov->disphi;

         printf("byte ");
         printMem(rm, 0);
         printf(", ");
         printf("%d", imm);

         if (simulate)
         {
            raiseUnimplemented();
         }
      }
      printf("\n");
   }

   else if (mod == 1)
   {
      uint8_t disp = mov->displo;
      int imm;
      int addr;
      int old_value;
      int new_value;

      printf("mov ");
      if (w)
      {
         printf("word ");
         instr_size = 5;
         imm = *(int16_t *)&mov->disphi;
         if (simulate)
         {
            addr = getEffectiveAddress(rm, disp);
            old_value = *(uint16_t *)&memory[addr];
            *(uint16_t *)&memory[addr] = imm;
            new_value = *(uint16_t *)&memory[addr];
         }
      }
      else
      {
         printf("byte ");
         instr_size = 4;
         imm = mov->disphi;
         if (simulate)
         {
            addr = getEffectiveAddress(rm, disp);
            old_value = memory[addr];
            memory[addr] = imm;
            new_value = memory[addr];
         }
      }

      printMem(rm, disp);
      printf(", %d", imm);
      if (simulate)
      {
         if (w)
            printf("; [0x%04x] -> [0x%04x]", old_value, new_value);
         else
            printf("; memory[%d]: [0x%02x] -> [0x%02x]", addr, old_value, new_value);
      }
      printf("\n");
   }

   else if (mod == 2)
   {
      printf("mov ");
      int disp = *(int16_t *)&mov->displo;
      int imm = 0;
      if (w)
      {
         instr_size = 6;
         imm = mov->datalo + 256 * mov->datahi;
         printf("word ");
      }
      else
      {
         instr_size = 5;
         imm = mov->datalo;
         printf("byte ");
      }
      printMem(rm, disp);
      printf(", ");
      printf("%d", imm);

      if (simulate)
      {
         int addr = getEffectiveAddress(rm, disp);
         if (w)
         {
            raiseUnimplemented();
         }
         else
         {
            uint8_t old_value = memory[addr];
            memory[addr] = imm;
            uint8_t new_value = memory[addr];
            printf("; memory[%d]: [0x%02x] -> [0x%02x]", addr, old_value, new_value);
         }
      }

      printf("\n");
   }

   else if (mod == 3)
   {
      raiseUnimplemented();
   }
   else
      assert(0);

   return instr_size;
}

struct ADDImmRegMem
{
   uint8_t opcode_sw;
   uint8_t mod_000_rm;
   uint8_t displo;
   uint8_t disphi;
   uint8_t datalo;
   uint8_t datahi;
};

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
      int imm = 0;
      if (s)
      {
         if (w)
         {
            instr_size = 3;
            imm = *(int8_t *)&add->displo;
            printf("word ");
         }
         else
            raiseUnimplemented();
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
      raiseUnimplemented();
   }

   else if (mod == 2)
   {
      int disp = *(int16_t *)&add->displo;
      int imm = 0;
      if (s)
      {
         if (w)
         {
            instr_size = 5;
            imm = *(int8_t *)&add->datalo;
            printf("word ");
         }
         else
            raiseUnimplemented();
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
      int imm = 0;
      if (s)
      {
         if (w)
         {
            instr_size = 3;
            imm = *(int8_t *)&add->displo;
         }
         else
            raiseUnimplemented();
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
      printf(", %d", imm);

      if (simulate)
      {
         int old_value = getReg(rm, w);
         setReg(rm, w, old_value + imm);
         int new_value = getReg(rm, w);
         setFlags(new_value, w);
         printf("; 0x%x -> 0x%x, flags = ", old_value, new_value);
         printFlags();
      }

      if (showclockcycles)
      {
         int clocks = 4;
         totalcycles += clocks;
         printf("; %d clocks, total = %d", clocks, totalcycles);
      }

      printf("\n");
   }
   return instr_size;
}

struct SUBImmRegMem
{
   uint8_t opcode_sw;
   uint8_t mod_101_rm;
   uint8_t displo;
   uint8_t disphi;
   uint8_t datalo;
   uint8_t datahi;
};

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
      int imm = 0;
      if (s)
      {
         if (w)
         {
            instr_size = 3;
            imm = *(int8_t *)&sub->displo;
            printf("word ");
         }
         else
            raiseUnimplemented();
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
      raiseUnimplemented();
   }

   else if (mod == 2)
   {
      int disp = *(int16_t *)&sub->displo;
      int imm = 0;
      if (s)
      {
         if (w)
         {
            instr_size = 5;
            imm = *(int8_t *)&sub->datalo;
            printf("word ");
         }
         else
            raiseUnimplemented();
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
      int imm = 0;
      if (s)
      {
         if (w)
         {
            instr_size = 3;
            imm = *(int8_t *)&sub->displo;
         }
         else
            raiseUnimplemented();
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
      printf(", %d", imm);

      if (simulate)
      {
         int old_value = getReg(rm, w);
         setReg(rm, w, old_value - imm);
         int new_value = getReg(rm, w);
         setFlags(new_value, w);
         printf("; 0x%x -> 0x%x, flags = ", old_value, new_value);
         printFlags();
      }

      printf("\n");
   }
   return instr_size;
}

struct CMPImmRegMem
{
   uint8_t opcode_sw;
   uint8_t mod_111_rm;
   uint8_t displo;
   uint8_t disphi;
   uint8_t datalo;
   uint8_t datahi;
};

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
         int imm = 0;
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
         int imm = 0;
         if (s)
         {
            if (w)
            {
               instr_size = 3;
               imm = *(int8_t *)&cmp->displo;
               printf("word ");
            }
            else
               raiseUnimplemented();
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
      raiseUnimplemented();
   }

   else if (mod == 2)
   {
      int disp = *(int16_t *)&cmp->displo;
      int imm = 0;
      if (s)
      {
         if (w)
         {
            instr_size = 5;
            imm = *(int8_t *)&cmp->datalo;
            printf("word ");
         }
         else
            raiseUnimplemented();
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
      int imm = 0;
      if (s)
      {
         if (w)
         {
            instr_size = 3;
            imm = *(int8_t *)&cmp->displo;
         }
         else
            raiseUnimplemented();
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
      printf(", %d", imm);

      if (simulate)
      {
         setFlags(getReg(rm, w) - imm, w);
         printf("; ");
         printFlags();
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
   int imm = 0;
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
   int imm = 0;
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
   int imm = 0;
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

struct JE
{
   uint8_t opcode;
   int8_t offset;
};

int decode_JE(struct JE *je)
{
   assert(je->opcode == 0x74);
   printf("je %d\n", je->offset);
   if (simulate && flags.ZF)
      registers.ip += je->offset;
   int instr_size = 2;
   return instr_size;
}

struct JNZ
{
   uint8_t opcode;
   int8_t offset;
};

int decode_JNZ(struct JNZ *jnz)
{
   assert(jnz->opcode == 0x75);
   printf("jnz %d", jnz->offset);
   if (simulate)
   {
      if (!flags.ZF)
      {
         registers.ip += jnz->offset;
         printf("; taken");
      }
      else
         printf("; not taken");
   }
   printf("\n");
   int instr_size = 2;
   return instr_size;
}

struct JL
{
   uint8_t opcode;
   int8_t offset;
};

int decode_JL(struct JL *jl)
{
   assert(jl->opcode == 0x7c);
   printf("jl %d\n", jl->offset);
   if (simulate && flags.SF)
      registers.ip += jl->offset;
   int instr_size = 2;
   return instr_size;
}

struct JLE
{
   uint8_t opcode;
   int8_t offset;
};

int decode_JLE(struct JLE *jle)
{
   assert(jle->opcode == 0x7e);
   printf("jle %d\n", jle->offset);
   if (simulate && (flags.ZF || flags.SF))
      registers.ip += jle->offset;
   int instr_size = 2;
   return instr_size;
}

struct JB
{
   uint8_t opcode;
   int8_t offset;
};

int decode_JB(struct JB *jb)
{
   assert(jb->opcode == 0x72);
   printf("jb %d\n", jb->offset);
   int instr_size = 2;
   return instr_size;
}

struct JBE
{
   uint8_t opcode;
   int8_t offset;
};

int decode_JBE(struct JBE *jbe)
{
   assert(jbe->opcode == 0x76);
   printf("jbe %d\n", jbe->offset);
   int instr_size = 2;
   return instr_size;
}

struct JP
{
   uint8_t opcode;
   int8_t offset;
};

int decode_JP(struct JP *jp)
{
   assert(jp->opcode == 0x7a);
   printf("jp %d\n", jp->offset);
   int instr_size = 2;
   return instr_size;
}

struct JO
{
   uint8_t opcode;
   int8_t offset;
};

int decode_JO(struct JO *jo)
{
   assert(jo->opcode == 0x70);
   printf("jo %d\n", jo->offset);
   int instr_size = 2;
   return instr_size;
}

struct JS
{
   uint8_t opcode;
   int8_t offset;
};

int decode_JS(struct JS *js)
{
   assert(js->opcode == 0x78);
   printf("js %d\n", js->offset);
   if (simulate && flags.SF)
      registers.ip += js->offset;
   int instr_size = 2;
   return instr_size;
}

struct JGE
{
   uint8_t opcode;
   int8_t offset;
};

int decode_JGE(struct JGE *jge)
{
   assert(jge->opcode == 0x7d);
   printf("jge %d\n", jge->offset);
   if (simulate && (flags.ZF || !flags.SF))
      registers.ip += jge->offset;
   int instr_size = 2;
   return instr_size;
}

struct JG
{
   uint8_t opcode;
   int8_t offset;
};

int decode_JG(struct JG *jg)
{
   assert(jg->opcode == 0x7f);
   printf("jg %d\n", jg->offset);
   if (simulate && !flags.SF)
      registers.ip += jg->offset;
   int instr_size = 2;
   return instr_size;
}

struct JNB
{
   uint8_t opcode;
   int8_t offset;
};

int decode_JNB(struct JNB *jnb)
{
   assert(jnb->opcode == 0x73);
   printf("jnb %d\n", jnb->offset);
   int instr_size = 2;
   return instr_size;
}

struct JA
{
   uint8_t opcode;
   int8_t offset;
};

int decode_JA(struct JA *ja)
{
   assert(ja->opcode == 0x77);
   printf("ja %d\n", ja->offset);
   int instr_size = 2;
   return instr_size;
}

struct JNP
{
   uint8_t opcode;
   int8_t offset;
};

int decode_JNP(struct JNP *jnp)
{
   assert(jnp->opcode == 0x7b);
   printf("jnp %d\n", jnp->offset);
   int instr_size = 2;
   return instr_size;
}

struct JNO
{
   uint8_t opcode;
   int8_t offset;
};

int decode_JNO(struct JNO *jno)
{
   assert(jno->opcode == 0x71);
   printf("jno %d\n", jno->offset);
   int instr_size = 2;
   return instr_size;
}

struct JNS
{
   uint8_t opcode;
   int8_t offset;
};

int decode_JNS(struct JNS *jns)
{
   assert(jns->opcode == 0x79);
   printf("jns %d\n", jns->offset);
   if (simulate && !flags.SF)
      registers.ip += jns->offset;
   int instr_size = 2;
   return instr_size;
}

struct LOOP
{
   uint8_t opcode;
   int8_t offset;
};

int decode_LOOP(struct LOOP *loop)
{
   assert(loop->opcode == 0xe2);
   printf("loop %d", loop->offset);
   if (simulate)
   {
      int old_value = registers.cx;
      registers.cx--;
      int new_value = registers.cx;
      printf("; cx: 0x%x -> 0x%x", old_value, new_value);
      if (registers.cx)
      {
         registers.ip += loop->offset;
      }
      else
         printf(", terminated");
   }
   printf("\n");
   int instr_size = 2;
   return instr_size;
}

struct LOOPZ
{
   uint8_t opcode;
   int8_t offset;
};

int decode_LOOPZ(struct LOOPZ *loopz)
{
   assert(loopz->opcode == 0xe1);
   printf("loopz %d\n", loopz->offset);
   int instr_size = 2;
   return instr_size;
}

struct LOOPNZ
{
   uint8_t opcode;
   int8_t offset;
};

int decode_LOOPNZ(struct LOOPNZ *loopnz)
{
   assert(loopnz->opcode == 0xe0);
   printf("loopnz %d\n", loopnz->offset);
   int instr_size = 2;
   return instr_size;
}

struct JCXZ
{
   uint8_t opcode;
   int8_t offset;
};

int decode_JCXZ(struct JCXZ *jcxz)
{
   assert(jcxz->opcode == 0xe3);
   printf("jcxz %d\n", jcxz->offset);
   int instr_size = 2;
   return instr_size;
}

struct MOVRegMemToSegment
{
   uint8_t opcode;
   uint8_t mod0SRrm;
   uint8_t displo;
   uint8_t disphi;
};

int decode_MOVRegMemToSegment(struct MOVRegMemToSegment *mov)
{
   int instr_size = 0;
   assert(mov->opcode == 0x8e);
   int mod = mov->mod0SRrm >> 6;
   int SR = (mov->mod0SRrm >> 3) & 3;
   int rm = mov->mod0SRrm & 7;
   assert(mod == 3); // other cases are unimplemented
   instr_size = 2;

   printf("mov ");
   print(SegmentRegisterNames[SR]);
   printf(", ");
   print(RegisterNames[rm][1]);

   if (simulate)
   {
      int old_value = getSegmentReg(SR);
      setSegmentReg(SR, getReg(rm, 1));
      int new_value = getSegmentReg(SR);
      printf("; 0x%x -> 0x%x", old_value, new_value);
   }
   printf("\n");
   return instr_size;
}

struct MOVSegmentToRegMem
{
   uint8_t opcode;
   uint8_t mod0SRrm;
   uint8_t displo;
   uint8_t disphi;
};

int decode_MOVSegmentToRegMem(struct MOVSegmentToRegMem *mov)
{
   int instr_size = 0;
   assert(mov->opcode == 0x8c);
   int mod = mov->mod0SRrm >> 6;
   int SR = (mov->mod0SRrm >> 3) & 3;
   int rm = mov->mod0SRrm & 7;
   assert(mod == 3); // other cases are unimplemented
   instr_size = 2;

   printf("mov ");
   print(RegisterNames[rm][1]);
   printf(", ");
   print(SegmentRegisterNames[SR]);

   if (simulate)
   {
      int old_value = getReg(rm, 1);
      setReg(rm, 1, getSegmentReg(SR));
      int new_value = getReg(rm, 1);
      printf("; 0x%x -> 0x%x", old_value, new_value);
   }
   printf("\n");
   return instr_size;
}

void decode_instruction(uint8_t *instruction)
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

   else if (*instruction == 0x74)
      instr_size = decode_JE((struct JE *)instruction);

   else if (*instruction == 0x75)
      instr_size = decode_JNZ((struct JNZ *)instruction);

   else if (*instruction == 0x7c)
      instr_size = decode_JL((struct JL *)instruction);

   else if (*instruction == 0x7e)
      instr_size = decode_JLE((struct JLE *)instruction);

   else if (*instruction == 0x72)
      instr_size = decode_JB((struct JB *)instruction);

   else if (*instruction == 0x76)
      instr_size = decode_JBE((struct JBE *)instruction);

   else if (*instruction == 0x7a)
      instr_size = decode_JP((struct JP *)instruction);

   else if (*instruction == 0x70)
      instr_size = decode_JO((struct JO *)instruction);

   else if (*instruction == 0x78)
      instr_size = decode_JS((struct JS *)instruction);

   else if (*instruction == 0x7d)
      instr_size = decode_JGE((struct JGE *)instruction);

   else if (*instruction == 0x7f)
      instr_size = decode_JG((struct JG *)instruction);

   else if (*instruction == 0x73)
      instr_size = decode_JNB((struct JNB *)instruction);

   else if (*instruction == 0x77)
      instr_size = decode_JA((struct JA *)instruction);

   else if (*instruction == 0x7b)
      instr_size = decode_JNP((struct JNP *)instruction);

   else if (*instruction == 0x71)
      instr_size = decode_JNO((struct JNO *)instruction);

   else if (*instruction == 0x79)
      instr_size = decode_JNS((struct JNS *)instruction);

   else if (*instruction == 0xe2)
      instr_size = decode_LOOP((struct LOOP *)instruction);

   else if (*instruction == 0xe1)
      instr_size = decode_LOOPZ((struct LOOPZ *)instruction);

   else if (*instruction == 0xe0)
      instr_size = decode_LOOPNZ((struct LOOPNZ *)instruction);

   else if (*instruction == 0xe3)
      instr_size = decode_JCXZ((struct JCXZ *)instruction);

   else if (*instruction == 0x8e)
      instr_size = decode_MOVRegMemToSegment((struct MOVRegMemToSegment *)instruction);

   else if (*instruction == 0x8c)
      instr_size = decode_MOVSegmentToRegMem((struct MOVSegmentToRegMem *)instruction);

   else
   {
      fprintf(stderr, "unimplemented opcode; first byte = 0x%x\n", *instruction);
      fprintf(stderr, "offset = %d\n", registers.ip);
      exit(1);
   }

   registers.ip += instr_size;
   if (registers.ip < 0 || instr_size == 0)
   {
      fprintf(stderr, "ip error\n");
      printRegisters();
      exit(1);
   }
}

int main(int argc, char **argv)
{
   char *objectfile;
   if (argc == 1)
   {
      puts("need objectfile for the 1st argument");
      exit(1);
   }

   if (argc > 2)
   {
      for (int i = 1; i < argc-1; i += 1)
      {
         if (strncmp(argv[i], "-exec", 5) == 0)
            simulate = true;
         else if (strncmp(argv[i], "-cycles", 7) == 0)
            showclockcycles = true;
      }
   }

   objectfile = argv[argc-1];

   FILE *f = fopen(objectfile, "r");
   assert(f);

   int codesize = readentirefile(f, memory, MEMORYSIZE);
   puts("bits 16");
   assert(registers.ip == 0);
   while (registers.ip < codesize)
   {
      decode_instruction(&memory[registers.ip]);
   }

   if (simulate)
   {
      printf("\n; Final CPU registers\n");
      printRegisters();
      printf("; flags = ");
      printFlags();
      printf("\n");

      /*
      puts("; ==== Final non-zero memory ====");
      for (int i = 0; i < MEMORYSIZE; i++)
      {
         if (memory[i])
            printf("; memory[%d] = 0x%02x\n", i, memory[i]);
      }
      */

      FILE *outputfile = fopen("memorydump.data", "w");
      assert(outputfile);
      dumpmemory(outputfile);
   }

   return 0;
}
