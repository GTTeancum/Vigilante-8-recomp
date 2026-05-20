# addr: 0x800159b4  name: FUN_800159b4
800159b4:  addiu sp,sp,-0x18
800159b8:  sw s0,0x10(sp)
800159bc:  sw ra,0x14(sp)
800159c0:  jal 0x800157d4
800159c4:  _move s0,a0
800159c8:  beq v0,zero,0x800159e8
800159cc:  _nop
800159d0:  lw a0,0xc(v0)
800159d4:  jal 0x8001570c
800159d8:  _nop
800159dc:  sw zero,0x6ac(gp)
800159e0:  j 0x800159f0
800159e4:  _li v0,0x1
800159e8:  jal 0x80015368
800159ec:  _move a0,s0
800159f0:  lw ra,0x14(sp)
800159f4:  lw s0,0x10(sp)
800159f8:  jr ra
800159fc:  _addiu sp,sp,0x18
