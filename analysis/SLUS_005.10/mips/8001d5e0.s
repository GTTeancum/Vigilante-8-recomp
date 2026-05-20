# addr: 0x8001d5e0  name: FUN_8001d5e0
8001d5e0:  addiu sp,sp,-0x18
8001d5e4:  sw ra,0x10(sp)
8001d5e8:  lw v0,0x3c(a0)
8001d5ec:  nop
8001d5f0:  beq v0,zero,0x8001d614
8001d5f4:  _nop
8001d5f8:  jal 0x8001d5a0
8001d5fc:  _nop
8001d600:  move a0,v0
8001d604:  lw v0,0x3c(a0)
8001d608:  nop
8001d60c:  bne v0,zero,0x8001d5f8
8001d610:  _nop
8001d614:  lw ra,0x10(sp)
8001d618:  move v0,a0
8001d61c:  jr ra
8001d620:  _addiu sp,sp,0x18
