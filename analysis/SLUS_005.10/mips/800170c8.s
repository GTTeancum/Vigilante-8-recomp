# addr: 0x800170c8  name: FUN_800170c8
800170c8:  addiu sp,sp,-0x18
800170cc:  sra v0,a1,0x0
800170d0:  sra v1,a1,0x1f
800170d4:  sw ra,0x14(sp)
800170d8:  sw s0,0x10(sp)
800170dc:  gte_ldLZCS v0
800170e0:  nop
800170e4:  nop
800170e8:  gte_stLZCR v0
800170ec:  li s0,0x23
800170f0:  subu s0,s0,v0
800170f4:  sra s0,s0,0x1
800170f8:  sll v0,s0,0x1
800170fc:  sll v1,v0,0x1a
80017100:  bgez v1,0x80017114
80017104:  _nop
80017108:  srav t0,a1,v0
8001710c:  bgez zero,0x8001712c
80017110:  _sra t1,a1,0x1f
80017114:  beq v1,zero,0x80017128
80017118:  _srlv t0,a0,v0
8001711c:  subu v1,zero,v0
80017120:  sllv v1,a1,v1
80017124:  or t0,t0,v1
80017128:  srav t1,a1,v0
8001712c:  move a0,t0
80017130:  jal 0x8004c6e4
80017134:  _move a1,t1
80017138:  lw ra,0x14(sp)
8001713c:  sllv v0,v0,s0
80017140:  lw s0,0x10(sp)
80017144:  jr ra
80017148:  _addiu sp,sp,0x18
