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

void
print_state()
{
  fprintf(ofp, "STACK: ");
  for (u16 i = SP; i < STACK_S; i++)
    fprintf(ofp, "%d ", stack[i]);
  fprintf(ofp, "\n");
  fprintf(ofp, "REGISTERS: ");
  for (u16 i = 0; i < NUM_OF_REGISTERS; i++)
    fprintf(ofp, "%d ", registers[i]);
  fprintf(ofp, "\n");
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
      fprintf(ofp, "\t%d: halt\n", PC);
      break;
    /*
     * set: 1 a b
     *   set register <a> to the value of <b>
     */
    case set: {
      fprintf(ofp, "\t%d: set %d, %d\n", PC, a, b);
      PC += 2;
      break;
    }
    /*
     * push <a> onto the stack
     */
    case push:
      fprintf(ofp, "\t%d: push %d\n", PC, a);
      PC++;
      break;
    /*
     * remove the top element from the stack and write it into <a>; empty stack
     * = error
     */
    case pop: {
      fprintf(ofp, "\t%d: pop %d\n", PC, a);
      PC++;
      break;
    }
    /*
     * eq a b c: set <a> to 1 if <b> is equal to <c>; set it to 0 otherwise
     */
    case eq: {
      fprintf(ofp, "\t%d: eq %d, %d, %d\n", PC, a, b, c);
      PC += 3;
      break;
    }
    /*
     * gt a b c: set <a> to 1 if <b> is greater than <c>; set it to 0 otherwise
     */
    case gt: {
      fprintf(ofp, "\t%d: gt %d, %d, %d\n", PC, a, b, c);
      PC += 3;
      break;
    }
    /*
     * jmp a: jump to <a>
     */
    case jmp: {
      fprintf(ofp, "\t%d: jump %d\n", PC, a);
      PC++;
      break;
    }
    /*
     * jt a b: if <a> is nonzero, jump to <b>
     */
    case jt: {
      fprintf(ofp, "\t%d: jt %d, %d\n", PC, a, b);
      PC += 2;
      break;
    }
    /*
     * jf a b: if <a> is zero, jump to <b>
     */
    case jf: {
      fprintf(ofp, "\t%d: jf %d, %d\n", PC, a, b);
      PC += 2;
      break;
    }
    /*
     * add a b c: assign into <a> the sum of <b> and <c> (modulo 32768)
     */
    case add: {
      fprintf(ofp, "\t%d: eq %d, %d, %d\n", PC, a, b, c);
      PC += 3;
      break;
    }
    /*
     * mult a b c: store into <a> the product of <b> and <c> (modulo 32768)
     */
    case mult: {
      fprintf(ofp, "\t%d: mult %d, %d, %d\n", PC, a, b, c);
      PC += 3;
      break;
    }
    /*
     * mod a b c: store into <a> the remainder of <b> divided by <c>
     */
    case mod: {
      fprintf(ofp, "\t%d: mod %d, %d, %d\n", PC, a, b, c);
      PC += 3;
      break;
    }
    /*
     * and a b c: stores into <a> the bitwise and of <b> and <c>
     */
    case and: {
      fprintf(ofp, "\t%d: and %d, %d, %d\n", PC, a, b, c);
      PC += 3;
      break;
    }
    /*
     * or a b c: stores into <a> the bitwise or of <b> and <c>
     */
    case or: {
      fprintf(ofp, "\t%d: or %d, %d, %d\n", PC, a, b, c);
      PC += 3;
      break;
    }
    /*
     * not a b: stores 15-bit bitwise inverse of <b> in <a>
     */
    case not: {
      fprintf(ofp, "\t%d: not %d, %d\n", PC, a, b);
      PC += 2;
      break;
    }
    /*
     * rmem a b: read memory at address <b> and write it to <a>
     */
    case rmem: {
      fprintf(ofp, "\t%d: rmem %d, %d\n", PC, a, b);
      PC += 2;
      break;
    }
    /*
     * wmem a b: write the value from <b> into memory at address <a>
     */
    case wmem: {
      fprintf(ofp, "\t%d: wmem %d, %d\n", PC, a, b);
      PC += 2;
      break;
    }
    /*
     * call a: write the address of the next instruction to the stack and jump
     * to <a>
     */
    case call: {
      fprintf(ofp, "\t%d: call %d\n", PC, a);
      PC++;
      break;
    }
    /*
     * ret: remove the top element from the stack and jump to it; empty stack =
     * halt
     */
    case ret:
      fprintf(ofp, "\t%d: ret\n", PC);
      break;
    /*
     * out: 19 a
     * write the character represented by ascii code <a> to the terminal
     */
    case out: {
      fprintf(ofp, "\t%d: out %d\n", PC, a);
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
      fprintf(ofp, "\t%d: in %d\n", PC, a);
      PC++;
      break;
    }
    case noop:
        fprintf(ofp, "\t%d: noop\n", PC);
      break;
    default:
        fprintf(ofp, "\t%d: wrong op %d\n", PC, ram[PC]);
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
  int acc = 29450;
  while (acc > 0) {
    eval(fetch(), ram[PC + 1], ram[PC + 2], ram[PC + 3]);
    PC++;
    acc--;
  }
  fclose(fp);
  fclose(ofp);
}
