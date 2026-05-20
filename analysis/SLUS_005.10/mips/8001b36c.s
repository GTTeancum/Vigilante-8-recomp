# addr: 0x8001b36c  name: FUN_8001b36c
8001b36c:  addiu sp,sp,-0x18
8001b370:  andi a1,a1,0xffff
8001b374:  sll v0,a1,0x1
8001b378:  addu v0,v0,a1
8001b37c:  sll v0,v0,0x2
8001b380:  addiu v0,v0,0xc
8001b384:  sw s0,0x10(sp)
8001b388:  addu s0,a0,v0
8001b38c:  sw ra,0x14(sp)
8001b390:  lhu v0,0x0(s0)
8001b394:  nop
8001b398:  bne v0,zero,0x8001b3c0
8001b39c:  _nop
8001b3a0:  lw v0,0x0(a0)
8001b3a4:  nop
8001b3a8:  lw v1,0x14(v0)
8001b3ac:  sll v0,a1,0x2
8001b3b0:  addu v0,v0,v1
8001b3b4:  lw a0,0x0(v0)
8001b3b8:  jal 0x800187e4
8001b3bc:  _move a1,s0
8001b3c0:  lw ra,0x14(sp)
8001b3c4:  move v0,s0
8001b3c8:  lw s0,0x10(sp)
8001b3cc:  jr ra
8001b3d0:  _addiu sp,sp,0x18
