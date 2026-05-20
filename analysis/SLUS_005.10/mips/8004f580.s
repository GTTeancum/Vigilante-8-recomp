# addr: 0x8004f580  name: DrawSync
8004f580:  lui v0,0x8006
8004f584:  lbu v0,0x5026(v0)
8004f588:  addiu sp,sp,-0x18
8004f58c:  sw s0,0x10(sp)
8004f590:  move s0,a0
8004f594:  sltiu v0,v0,0x2
8004f598:  bne v0,zero,0x8004f5bc
8004f59c:  _sw ra,0x14(sp)
8004f5a0:  lui a0,0x8001
8004f5a4:  addiu a0,a0,0x123c
8004f5a8:  lui v0,0x8006
8004f5ac:  lw v0,0x5020(v0)
8004f5b0:  nop
8004f5b4:  jalr v0
8004f5b8:  _move a1,s0
8004f5bc:  lui v0,0x8006
8004f5c0:  lw v0,0x501c(v0)
8004f5c4:  nop
8004f5c8:  lw v0,0x3c(v0)
8004f5cc:  nop
8004f5d0:  jalr v0
8004f5d4:  _move a0,s0
8004f5d8:  lw ra,0x14(sp)
8004f5dc:  lw s0,0x10(sp)
8004f5e0:  jr ra
8004f5e4:  _addiu sp,sp,0x18
