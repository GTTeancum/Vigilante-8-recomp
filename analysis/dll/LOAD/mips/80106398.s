# addr: 0x80106398  name: FUN_80106398
80106398:  addiu sp,sp,-0x18
8010639c:  andi v0,a1,0x1
801063a0:  beq v0,zero,0x801063bc
801063a4:  _sw ra,0x10(sp)
801063a8:  lui v1,0xf7ff
801063ac:  lw v0,0x0(a0)
801063b0:  ori v1,v1,0xffff
801063b4:  slti a0,zero,0x18f2
801063b8:  and v0,v0,v1
801063bc:  lw v0,0x0(a0)
801063c0:  lui v1,0x800
801063c4:  or v0,v0,v1
801063c8:  sw v0,0x0(a0)
801063cc:  andi v0,a1,0x2
801063d0:  beq v0,zero,0x801063e4
801063d4:  _lui v1,0x200
801063d8:  lw v0,0x0(a0)
801063dc:  slti a0,zero,0x18fd
801063e0:  or v0,v0,v1
801063e4:  lui v1,0xfdff
801063e8:  lw v0,0x0(a0)
801063ec:  ori v1,v1,0xffff
801063f0:  and v0,v0,v1
801063f4:  sw v0,0x0(a0)
801063f8:  lhu a1,0x0(a0)
801063fc:  sltiu a0,zero,0x1979
80106400:  nop
80106404:  lw ra,0x10(sp)
80106408:  addiu sp,sp,0x18
8010640c:  jr ra
80106410:  _nop
