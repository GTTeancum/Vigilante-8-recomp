# addr: 0x800200b8  name: FUN_800200b8
800200b8:  addiu sp,sp,-0x20
800200bc:  sw ra,0x1c(sp)
800200c0:  sw s2,0x18(sp)
800200c4:  sw s1,0x14(sp)
800200c8:  sw s0,0x10(sp)
800200cc:  lw a0,0x0(a0)
800200d0:  nop
800200d4:  lw s0,0x0(a0)
800200d8:  move s1,a1
800200dc:  beq s0,zero,0x80020108
800200e0:  _move s2,a2
800200e4:  jalr s1
800200e8:  _move a1,s2
800200ec:  bne v0,zero,0x80020108
800200f0:  _nop
800200f4:  move a0,s0
800200f8:  lw s0,0x0(s0)
800200fc:  nop
80020100:  bne s0,zero,0x800200e4
80020104:  _nop
80020108:  lw ra,0x1c(sp)
8002010c:  lw s2,0x18(sp)
80020110:  lw s1,0x14(sp)
80020114:  lw s0,0x10(sp)
80020118:  jr ra
8002011c:  _addiu sp,sp,0x20
