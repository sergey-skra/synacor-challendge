#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "./arch.h"

/*      == architecture ==      */
#define RAM_S 32768
#define STACK_S 512
#define MOD 32768
#define MAX_NUMBER 32767
#define MAX_REGSTR 32775

typedef uint16_t u16;

u16 ram[RAM_S];
u16 registers[NUM_OF_REGISTERS];
u16 stack[STACK_S];

#define PC registers[PC]
#define SP registers[SP]

FILE *fp, *ofp;
bool running = true; /*  automata state  */

u16
get_arg_value()
{
  u16 x;
  if ((x = ram[++PC]) <= MAX_NUMBER)
    return x;
  else if (x > MAX_REGSTR) {
    return x % MOD;
  } else
    return registers[x - MOD];
}


u16
fetch()
{
  return ram[PC];
}

void
eval(u16 instr, u16 a, u16 b, u16 c)
{
  switch (instr) {
    case halt:

      break;
    /*
     * set: 1 a b
     *   set register <a> to the value of <b>
     */
    case set: {
      printf("\tset %d, %d\n", a, b);
      PC += 2;
      break;
    }
    /*
     * push <a> onto the stack
     */
    case push:
      printf("%d: push %d\n", PC, a);
      PC ++;
      break;
    /*
     * remove the top element from the stack and write it into <a>; empty stack
     * = error
     */
    case pop: {
      printf("%d: pop %d\n", PC, a);
      PC ++;
      break;
    }
    /*
     * eq a b c: set <a> to 1 if <b> is equal to <c>; set it to 0 otherwise
     */
    case eq: {
      printf("\teq %d, %d, %d\n", a, b, c);
      PC += 3;
      break;
    }
    /*
     * gt a b c: set <a> to 1 if <b> is greater than <c>; set it to 0 otherwise
     */
    case gt: {
     printf("\tgt %d, %d, %d\n", a, b, c);
      PC += 3;
      break;
    }
    /*
     * jmp a: jump to <a>
     */
    case jmp: {
      printf("%d: jump %d\n", PC, a);
      PC++;
      break;
    }
    /*
     * jt a b: if <a> is nonzero, jump to <b>
     */
    case jt: {
      printf("%d: jt %d, %d\n",PC, a, b);
      PC += 2;
      break;
    }
    /*
     * jf a b: if <a> is zero, jump to <b>
     */
    case jf: {
      printf("%d: jf %d, %d\n",PC, a, b);
      PC += 2;
      break;
    }
    /*
     * add a b c: assign into <a> the sum of <b> and <c> (modulo 32768)
     */
    case add: {
      printf("\teq %d, %d, %d\n", a, b, c);
      PC += 3;
      break;
    }
    /*
     * mult a b c: store into <a> the product of <b> and <c> (modulo 32768)
     */
    case mult: {
      printf("\tmult %d, %d, %d\n", a, b, c);
      PC += 3;
      break;
    }
    /*
     * mod a b c: store into <a> the remainder of <b> divided by <c>
     */
    case mod: {
      printf("\tmod %d, %d, %d\n", a, b, c);
      PC += 3;
      break;
    }
    /*
     * and a b c: stores into <a> the bitwise and of <b> and <c>
     */
    case and: {
      printf("\tand %d, %d, %d\n", a, b, c);
      PC += 3;
      break;
    }
    /*
     * or a b c: stores into <a> the bitwise or of <b> and <c>
     */
    case or: {
      printf("\tor %d, %d, %d\n", a, b, c);
      PC += 3;
      break;
    }
    /*
     * not a b: stores 15-bit bitwise inverse of <b> in <a>
     */
    case not: {
      printf("\tnot %d, %d\n", a, b);
      PC += 2;
      break;
    }
    /*
     * rmem a b: read memory at address <b> and write it to <a>
     */
    case rmem: {
      printf("\trmem %d, %d\n", a, b);
      PC += 2;
      break;
    }
    /*
     * wmem a b: write the value from <b> into memory at address <a>
     */
    case wmem: {
      printf("\twmem %d, %d\n", a, b);
      PC += 2;
      break;
    }
    /*
     * call a: write the address of the next instruction to the stack and jump
     * to <a>
     */
    case call: {
      printf("%d: call %d\n", PC, a);
      PC++;
      break;
    }
    /*
     * ret: remove the top element from the stack and jump to it; empty stack =
     * halt
     */
    case ret:
      printf("%d: ret\n", PC);
      PC++;
      break;
    /*
     * out: 19 a
     * write the character represented by ascii code <a> to the terminal
     */
    case out: {
      printf("%c", a);
      PC++;
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
      printf("\tin %d\n", a);
      PC++;
      break;
    }
    case noop:

      break;
  }
}

int
main(int argc, char* argv[])
{
  if (argc != 2) {
    fprintf(stderr, "usage: %s [file]\n", argv[0]);
    exit(1);
  }
  
  if ((fp = fopen(argv[1], "rb")))
    (void)fread(ram, sizeof(ram), RAM_S, fp);
  else
    printf("No such file %s\n", argv[1]);

  ofp = fopen("file.s", "w");

  if (ofp == NULL) {
    fprintf(stderr, "Can't open output file %s!\n", "file");
    exit(1);
  }
  SP = STACK_S;
  int acc = 20000;
  while (acc > 0) {
    eval(fetch(), ram[PC + 1], ram[PC + 2], ram[PC + 3]);
    PC++;
    acc--;
  }
  fclose(fp);
fclose(ofp);
}
