# addr: 0x800185cc  name: FUN_800185cc
800185cc:  lw v0,0x4(a1)
800185d0:  nop
800185d4:  sw v0,0x0(a0)
800185d8:  andi v0,v0,0x8
800185dc:  beq v0,zero,0x800185fc
800185e0:  _addiu v0,a1,0xc
800185e4:  sw v0,0x4(a0)
800185e8:  addiu v0,a1,0x14
800185ec:  sw v0,0x8(a0)
800185f0:  lw v0,0x8(a1)
800185f4:  j 0x80018604
800185f8:  _addu a1,a1,v0
800185fc:  sw zero,0x8(a0)
80018600:  sw zero,0x4(a0)
80018604:  addiu v0,a1,0xc
80018608:  sw v0,0xc(a0)
8001860c:  addiu v0,a1,0x14
80018610:  jr ra
80018614:  _sw v0,0x10(a0)
