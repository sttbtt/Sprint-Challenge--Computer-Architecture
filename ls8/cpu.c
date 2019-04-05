#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cpu.h"

#define DATA_LEN 6

unsigned char cpu_ram_read(struct cpu *cpu, unsigned char address)
{
  return cpu->ram[address];
}

void cpu_ram_write(struct cpu *cpu, unsigned char address, unsigned char value)
{
  cpu->ram[address] = value;
}

void cpu_push(struct cpu *cpu, unsigned char value)
{
  cpu->reg[SP]--;
  cpu->ram[cpu->reg[SP]] = value;
}

unsigned char cpu_pop(struct cpu *cpu)
{
  unsigned char value = cpu->ram[cpu->reg[SP]];
  cpu->reg[SP]++;
  return value;
}

void trace(struct cpu *cpu)
{
    printf("%02X | ", cpu->PC);

    printf("%02X %02X %02X |",
        cpu_ram_read(cpu, cpu->PC),
        cpu_ram_read(cpu, cpu->PC + 1),
        cpu_ram_read(cpu, cpu->PC + 2));

    for (int i = 0; i < 8; i++) {
        printf(" %02X", cpu->reg[i]);
    }

    printf("\n");
}

/**
 * Load the binary bytes from a .ls8 source file into a RAM array
 */
void cpu_load(struct cpu *cpu, int argc, char *argv[])
{
  FILE *fp;
  char line[1024];
  int address = 0x00;

  if (argc != 2) {
      printf("usage: ls8 filename\n");
      exit(1);
  }

  fp = fopen(argv[1], "r");

    if (fp == NULL) {
        printf("Error opening file %s\n", argv[1]);
        exit(2);
    }

    while (fgets(line, 1024, fp) != NULL) {
        char *endptr;
        unsigned char val = strtoul(line, &endptr, 2);
        if (line == endptr) {
            // printf("SKIPPING: %s", line);
            continue;
        }

        // store in memory
        cpu->ram[address++] = val;
    }

    fclose(fp);
}

/**
 * ALU
 */
void alu(struct cpu *cpu, enum alu_op op, unsigned char regA, unsigned char regB)
{
  unsigned char *reg = cpu->reg;
  switch (op) {

    case ALU_ADD:
      reg[regA] += reg[regB];
      break;
    
    case ALU_SUB:
      reg[regA] -= reg[regB];
      break;

    case ALU_MUL:
      reg[regA] *= reg[regB];
      break;

    case ALU_DIV:
      reg[regA] /= reg[regB];
      break;
    
    // TODO: implement more ALU ops
  }
}

/**
 * Run the CPU
 */
void cpu_run(struct cpu *cpu)
{
  int running = 1; // True until we get a HLT instruction

  while (running) {
    // TODO
    // 1. Get the value of the current instruction (in address PC).
    unsigned char IR = cpu->ram[cpu->PC];

    // 2. Figure out how many operands this next instruction requires
    unsigned int operands = IR >> DATA_LEN;
    // printf("Operands: %d, IR: %d\n", operands, IR);

    // 3. Get the appropriate value(s) of the operands following this instruction
    unsigned char operandA = cpu->ram[(cpu->PC + 1)];
    unsigned char operandB = cpu->ram[(cpu->PC + 2)];

    // 4. switch() over it to decide on a course of action.
    // 5. Do whatever the instruction should do according to the spec.
    // trace(cpu);
    switch (IR)
    {
      case LDI:
        cpu->reg[operandA] = operandB;
        cpu->PC += operands + 1;
        break;

      case PRN:
        printf("%d\n", cpu->reg[operandA]);
        cpu->PC += operands + 1;
        break;

      case ADD:
        alu(cpu, ALU_ADD, operandA, operandB);
        cpu->PC += operands + 1;
        break;

      case SUB:
        alu(cpu, ALU_SUB, operandA, operandB);
        cpu->PC += operands + 1;
        break;

      case MUL:
        alu(cpu, ALU_MUL, operandA, operandB);
        cpu->PC += operands + 1;
        break;

      case DIV:
        alu(cpu, ALU_DIV, operandA, operandB);
        cpu->PC += operands + 1;
        break;

      case PUSH:
        cpu_push(cpu, cpu->reg[operandA]);
        // cpu->reg[cpu->SP]--;
        // cpu_ram_write(cpu, cpu->reg[cpu->SP], cpu->reg[operandA]);
        cpu->PC += operands + 1;
        break;

      case POP:
        cpu->reg[operandA] = cpu_pop(cpu);
        // cpu->reg[operandA & cpu->SP] = cpu_ram_read(cpu, cpu->reg[cpu->SP]);
        // cpu->reg[cpu->SP]++;
        cpu->PC += operands + 1;
        break;

      case CALL:
        cpu_push(cpu, cpu->PC + 2);
        cpu->PC = cpu->reg[operandA];
        break;

      case RET:
        cpu->PC = cpu_pop(cpu);
        break;

      case HLT:
        running = 0;
        break;

      default:
        break;
    }

    // 6. Move the PC to the next instruction.
    // cpu->PC += operands + 1;
  }
}

/**
 * Initialize a CPU struct
 */
void cpu_init(struct cpu *cpu)
{
  // TODO: Initialize the PC and other special reg
  cpu->PC = 0;
  cpu->reg[SP] = 0xF4;
  memset(cpu->reg, 0, sizeof(cpu->reg));
  memset(cpu->ram, 0, sizeof(cpu->ram));
}


