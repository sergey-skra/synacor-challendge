enum Instructions{
    halt = 0x00,
    set,
    push,
    pop,
    eq,
    gt,
    jmp,
    jt,
    jf,
    add = 0x09,
    mult = 0x0A,
    mod,
    and,
    or,
    not,
    rmem = 0x0F,
    wmem = 0x10,
    call,
    ret,
    out,
    in,
    noop = 0x15,
};

enum Registers{
    r0, r1, r2, r3, r4, r5, r6, r7,
    SP,
    PC,
    NUM_OF_REGISTERS
};
