#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "./arch.h"

/*      == architecture ==      ahtlUUovjfOq  XTOWXpvvqAwM*/
#define RAM_S 32768
#define STACK_S 512
#define MOD 32768
#define MAX_NUMBER 32767
#define MAX_REGSTR 32775

typedef uint16_t u16;

u16 ram[RAM_S];
u16 registers[NUM_OF_REGISTERS];
u16 stack[STACK_S];
u16 debug_ram[RAM_S + NUM_OF_REGISTERS + STACK_S];

#define PC registers[PC]
#define SP registers[SP]

bool running = true; /*  automata states  */

void
load_state()
{
  FILE* ifp = fopen("dbfile", "rb");
  fread(debug_ram, sizeof(u16), sizeof(debug_ram), ifp);
  for (u16 i = 0; i < RAM_S; i++)
    ram[i] = debug_ram[i];
  for (u16 i = 0; i < NUM_OF_REGISTERS; i++)
    registers[i] = debug_ram[RAM_S + i];
  for (u16 i = 0; i < STACK_S; i++)
    stack[i] = debug_ram[RAM_S + NUM_OF_REGISTERS + i];
  fclose(ifp);
  printf("state loaded\n");
}

void
save_state()
{
  FILE* f = fopen("dbfile", "wb");

  for (u16 i = 0; i < RAM_S; i++)
    debug_ram[i] = ram[i];
  for (u16 i = 0; i < NUM_OF_REGISTERS; i++)
    debug_ram[RAM_S + i] = registers[i];
  for (u16 i = 0; i < STACK_S; i++)
    debug_ram[RAM_S + NUM_OF_REGISTERS + i] = stack[i];

  fwrite(debug_ram, sizeof(u16), sizeof(debug_ram), f);
  fclose(f);
  printf("state saved\n");
}

void
reg()
{

  registers[r7] = 32767;
  ram[6029] = 6030;
  ram[6030] = 1;
  ram[6032] = 6;
  ram[6033] = 21;

}

void
debug()
{
  char str[20];
  bool debuging = true;

  while (debuging) {
    printf(">");
    scanf("%s", str);
    if (strcmp(str, "exit") == 0)
      debuging = false;
    else if (strcmp(str, "load") == 0)
      load_state();
    else if (strcmp(str, "save") == 0)
      save_state();
    else if (strcmp(str, "reg") == 0)
      reg();
    else {
      printf("unknown command %s\n", str);
      printf("try load/save/exit\n");
    }
  }
}

u16
get_arg_value()
{
  u16 x;
  if ((x = ram[++PC]) <= MAX_NUMBER)
    return x;
  else if (x > MAX_REGSTR) {
    /*
     *    check this for errors
     */
    return x % MOD;
  } else
    return registers[x - MOD];
}

u16*
get_arg_adr()
{
  u16 x;
  u16* p;
  if ((x = ram[++PC]) <= MAX_NUMBER)
    return p = &ram[x];
  else if (x > MAX_REGSTR) {
    return p = &ram[x % MOD];
  } else
    return p = &registers[x - MOD];
}

void
print_state()
{
  printf("STACK: ");
  for (u16 i = SP; i < STACK_S; i++)
    printf("%d ", stack[i]);
  printf("\n");
  printf("REGISTERS: ");
  for (u16 i = 0; i < NUM_OF_REGISTERS; i++)
    printf("%d ", registers[i]);
  printf("\n");
}

u16
fetch()
{
  return ram[PC];
}

void
eval(u16 instr)
{
  switch (instr) {
    case halt:
      printf("halted");
      running = false;
      break;
    /*
     * set: 1 a b
     *   set register <a> to the value of <b>
     */
    case set: {
      u16* a = get_arg_adr();
      u16 b = get_arg_value();
      *a = b;

      break;
    }
    /*
     * push <a> onto the stack
     */
    case push:
      SP--;
      stack[SP] = get_arg_value();
      break;
    /*
     * remove the top element from the stack and write it into <a>; empty stack
     * = error
     */
    case pop: {
      if (SP == STACK_S)
        printf("Stack underflow! SP = %d PC = %d", SP, PC);
      u16* a = get_arg_adr();
      *a = stack[SP];
      SP++;
      break;
    }
    /*
     * eq a b c: set <a> to 1 if <b> is equal to <c>; set it to 0 otherwise
     */
    case eq: {
      u16* a = get_arg_adr();
      u16 b = get_arg_value();
      u16 c = get_arg_value();
      *a = 0;
      if (b == c)
        *a = 1;
      break;
    }
    /*
     * gt a b c: set <a> to 1 if <b> is greater than <c>; set it to 0 otherwise
     */
    case gt: {
      u16* a = get_arg_adr();
      u16 b = get_arg_value();
      u16 c = get_arg_value();
      *a = 0;
      if (b > c)
        *a = 1;
      break;
    }
    /*
     * jmp a: jump to <a>
     */
    case jmp: {
      u16 a = get_arg_value();
      PC = a - 1;
      break;
    }
    /*
     * jt a b: if <a> is nonzero, jump to <b>
     */
    case jt: {
      u16 a = get_arg_value();
      u16 b = get_arg_value();
      if (a != 0)
        PC = b - 1;
      break;
    }
    /*
     * jf a b: if <a> is zero, jump to <b>
     */
    case jf: {
      u16 a = get_arg_value();
      u16 b = get_arg_value();
      if (a == 0)
        PC = b - 1;
      break;
    }
    /*
     * add a b c: assign into <a> the sum of <b> and <c> (modulo 32768)
     */
    case add: {
      u16* a = get_arg_adr();
      u16 b = get_arg_value();
      u16 c = get_arg_value();
      *a = (b + c) % 32768;
      break;
    }
    /*
     * mult a b c: store into <a> the product of <b> and <c> (modulo 32768)
     */
    case mult: {
      u16* a = get_arg_adr();
      u16 b = get_arg_value();
      u16 c = get_arg_value();
      *a = (b * c) % 32768;
      break;
    }
    /*
     * mod a b c: store into <a> the remainder of <b> divided by <c>
     */
    case mod: {
      u16* a = get_arg_adr();
      u16 b = get_arg_value();
      u16 c = get_arg_value();
      *a = b % c;
      break;
    }
    /*
     * and a b c: stores into <a> the bitwise and of <b> and <c>
     */
    case and: {
      u16* a = get_arg_adr();
      u16 b = get_arg_value();
      u16 c = get_arg_value();
      *a = b & c;
      break;
    }
    /*
     * or a b c: stores into <a> the bitwise or of <b> and <c>
     */
    case or: {
      u16* a = get_arg_adr();
      u16 b = get_arg_value();
      u16 c = get_arg_value();
      *a = b | c;
      break;
    }
    /*
     * not a b: stores 15-bit bitwise inverse of <b> in <a>
     */
    case not: {
      u16* a = get_arg_adr();
      u16 b = get_arg_value();
      *a = b ^ 0b0111111111111111;
      break;
    }
    /*
     * rmem a b: read memory at address <b> and write it to <a>
     */
    case rmem: {
      u16* a = get_arg_adr();
      u16 b = get_arg_value();
      *a = ram[b];
      break;
    }
    /*
     * wmem a b: write the value from <b> into memory at address <a>
     */
    case wmem: {
      u16 a = get_arg_value();
      u16 b = get_arg_value();
      ram[a] = b;
      break;
    }
    /*
     * call a: write the address of the next instruction to the stack and jump
     * to <a>
     */
    case call: {
      u16 a = get_arg_value();
      SP--;
      stack[SP] = ++PC;
      PC = a - 1;
      break;
    }
    /*
     * ret: remove the top element from the stack and jump to it; empty stack =
     * halt
     */
    case ret:
      if (SP == STACK_S) {
        printf("err ret: empty stack");
        running = false;
        break;
      }
      u16 a = stack[SP];
      SP++;
      PC = a - 1;
      break;
    /*
     * out: 19 a
     * write the character represented by ascii code <a> to the terminal
     */
    case out: {
      u16 a = get_arg_value();
      printf("%c", a);
      break;
    }
    /*
     * in: 20 a
     * read a character from the terminal and write its ascii code to <a>;
     * it can be assumed that once input starts, it will continue
     * until a newline is encountered; this means that you can safely read whole
     * lines from the keyboard and trust that they will be fully read
     */
    case in: {
      u16* a = get_arg_adr();
      char ch;
      scanf("%c", &ch);
      if (ch == '@')
        debug();
      *a = ch;
      break;
    }
    case noop:
      break;
    default:
      printf("Wrong operation: %d\n", instr);
      printf("PC = %d\n", PC);
      running = false;
  }
}

int
main(int argc, char* argv[])
{
  if (argc != 2) {
    fprintf(stderr, "usage: %s [file]\n", argv[0]);
    exit(1);
  }
  FILE* fp;
  if ((fp = fopen(argv[1], "rb")))
    (void)fread(ram, sizeof(ram), RAM_S, fp);
  else
    printf("No such file %s\n", argv[1]);

  while (running) {
    eval(fetch());
    PC++;
  }
}
