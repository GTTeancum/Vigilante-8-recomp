# addr: 0x80052584  name: strncmp
80052584:  beq a0,zero,0x80052594
80052588:  _nop
8005258c:  bne a1,zero,0x800525ac
80052590:  _nop
80052594:  beq a0,a1,0x800525fc
80052598:  _clear v0
8005259c:  beq a0,zero,0x800525fc
800525a0:  _li v0,-0x1
800525a4:  j 0x800525fc
800525a8:  _li v0,0x1
800525ac:  addiu a2,a2,-0x1
800525b0:  bltz a2,0x800525fc
800525b4:  _clear v0
800525b8:  lb v1,0x0(a0)
800525bc:  lb v0,0x0(a1)
800525c0:  nop
800525c4:  bne v1,v0,0x800525e0
800525c8:  _addiu a1,a1,0x1
800525cc:  beq v1,zero,0x800525f8
800525d0:  _addiu a0,a0,0x1
800525d4:  addiu a2,a2,-0x1
800525d8:  bgez a2,0x800525b8
800525dc:  _nop
800525e0:  bltz a2,0x800525f8
800525e4:  _nop
800525e8:  lb v1,0x0(a0)
800525ec:  lb v0,-0x1(a1)
800525f0:  j 0x800525fc
800525f4:  _subu v0,v1,v0
800525f8:  clear v0
