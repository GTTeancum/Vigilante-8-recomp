# addr: 0x800480b4  name: DMACallback
800480b4:  lui v0,0x8006
800480b8:  lw v0,-0x8c(v0)
800480bc:  addiu sp,sp,-0x18
800480c0:  sw ra,0x10(sp)
800480c4:  lw v0,0x4(v0)
800480c8:  nop
800480cc:  jalr v0
800480d0:  _nop
800480d4:  lw ra,0x10(sp)
800480d8:  addiu sp,sp,0x18
800480dc:  jr ra
800480e0:  _nop
