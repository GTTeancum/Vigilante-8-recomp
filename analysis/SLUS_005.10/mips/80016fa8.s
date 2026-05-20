# addr: 0x80016fa8  name: FUN_80016fa8
80016fa8:  addiu sp,sp,-0x38
80016fac:  sw ra,0x34(sp)
80016fb0:  sw s0,0x30(sp)
80016fb4:  lh v0,0x0(a1)
80016fb8:  nop
80016fbc:  subu v0,zero,v0
80016fc0:  sw v0,0x20(sp)
80016fc4:  lh v0,0x2(a1)
80016fc8:  nop
80016fcc:  subu v0,zero,v0
80016fd0:  sw v0,0x24(sp)
80016fd4:  lh v0,0x4(a1)
80016fd8:  nop
80016fdc:  subu v0,zero,v0
80016fe0:  sw v0,0x28(sp)
80016fe4:  lh v0,0x2(a1)
80016fe8:  nop
80016fec:  subu v0,zero,v0
80016ff0:  sw v0,0x10(sp)
80016ff4:  lh v0,0x0(a1)
80016ff8:  move s0,a0
80016ffc:  addiu a0,sp,0x10
80017000:  move a1,a0
80017004:  sw zero,0x18(sp)
80017008:  jal 0x8004c814
8001700c:  _sw v0,0x14(sp)
80017010:  addiu a0,sp,0x20
80017014:  jal 0x8004c814
80017018:  _move a1,a0
8001701c:  lw t4,0x10(sp)
80017020:  lw t5,0x14(sp)
80017024:  gte_ldR11R12 t4
80017028:  lw t4,0x18(sp)
8001702c:  gte_ldR22R23 t5
80017030:  gte_ldR33 t4
80017034:  gte_ldIR3 0x28(sp)
80017038:  gte_ldIR1 0x20(sp)
8001703c:  gte_ldIR2 0x24(sp)
80017040:  nOP12
8001704c:  lhu v0,0x10(sp)
80017050:  nop
80017054:  sh v0,0x0(s0)
80017058:  lhu v0,0x14(sp)
8001705c:  nop
80017060:  sh v0,0x6(s0)
80017064:  lhu v0,0x18(sp)
80017068:  nop
8001706c:  sh v0,0xc(s0)
80017070:  lhu v0,0x20(sp)
80017074:  nop
80017078:  sh v0,0x2(s0)
8001707c:  lhu v0,0x24(sp)
80017080:  nop
80017084:  sh v0,0x8(s0)
80017088:  lhu v0,0x28(sp)
8001708c:  nop
80017090:  sh v0,0xe(s0)
80017094:  gte_stMAC1 v0
80017098:  nop
8001709c:  sh v0,0x4(s0)
800170a0:  gte_stMAC2 v0
800170a4:  nop
800170a8:  sh v0,0xa(s0)
800170ac:  gte_stMAC3 v1
800170b0:  move v0,s0
800170b4:  sh v1,0x10(v0)
800170b8:  lw ra,0x34(sp)
800170bc:  lw s0,0x30(sp)
800170c0:  jr ra
800170c4:  _addiu sp,sp,0x38
