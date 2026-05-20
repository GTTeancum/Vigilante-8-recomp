# addr: 0x80106794  name: FUN_80106794
80106794:  addiu sp,sp,-0x20
80106798:  lui v1,0x8010
8010679c:  lw v1,0x6f20(v1)
801067a0:  lui v0,0x10
801067a4:  sw ra,0x18(sp)
801067a8:  sw v0,0x10(sp)
801067ac:  lw v0,0x0(v1)
801067b0:  lui v1,0x100
801067b4:  and v0,v0,v1
801067b8:  beq v0,zero,0x80106818
801067bc:  _clear v0
801067c0:  li a0,-0x1
801067c4:  lw v0,0x10(sp)
801067c8:  nop
801067cc:  addiu v0,v0,-0x1
801067d0:  sw v0,0x10(sp)
801067d4:  lw v0,0x10(sp)
801067d8:  nop
801067dc:  bne v0,a0,0x801067f8
801067e0:  _nop
801067e4:  lui a0,0x8010
801067e8:  sltiu a0,zero,0x1a10
801067ec:  addiu a0,a0,0x124
801067f0:  slti a0,zero,0x1a06
801067f4:  li v0,-0x1
801067f8:  lui v0,0x8010
801067fc:  lw v0,0x6f20(v0)
80106800:  nop
80106804:  lw v0,0x0(v0)
80106808:  nop
8010680c:  and v0,v0,v1
80106810:  bne v0,zero,0x801067c4
80106814:  _clear v0
80106818:  lw ra,0x18(sp)
8010681c:  addiu sp,sp,0x20
80106820:  jr ra
80106824:  _nop
