# addr: 0x80101390  name: FUN_80101390
80101390:  addiu sp,sp,-0x18
80101394:  sw s0,0x10(sp)
80101398:  sw ra,0x14(sp)
8010139c:  jal 0x80022c54
801013a0:  _move s0,a0
801013a4:  lui v1,0x8006
801013a8:  lw v1,0x59fc(v1)
801013ac:  nop
801013b0:  lw a1,0x80(v1)
801013b4:  nop
801013b8:  lw v1,0x0(a1)
801013bc:  nop
801013c0:  beq v1,zero,0x801013fc
801013c4:  _move a2,v0
801013c8:  lw a0,0x50(s0)
801013cc:  lw v0,0x8(a1)
801013d0:  nop
801013d4:  lw v0,0x50(v0)
801013d8:  nop
801013dc:  slt v0,v0,a0
801013e0:  beq v0,zero,0x801013fc
801013e4:  _nop
801013e8:  move a1,v1
801013ec:  lw v1,0x0(a1)
801013f0:  nop
801013f4:  bne v1,zero,0x801013cc
801013f8:  _nop
801013fc:  lw v0,0x4(a1)
80101400:  nop
80101404:  sw a2,0x0(v0)
80101408:  sw a2,0x4(a1)
8010140c:  sw a1,0x0(a2)
80101410:  sw v0,0x4(a2)
80101414:  lw ra,0x14(sp)
80101418:  lw s0,0x10(sp)
8010141c:  jr ra
80101420:  _addiu sp,sp,0x18
