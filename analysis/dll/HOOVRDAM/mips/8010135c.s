# addr: 0x8010135c  name: FUN_8010135c
8010135c:  addiu sp,sp,-0x18
80101360:  sw s0,0x10(sp)
80101364:  move s0,a0
80101368:  li v0,0x3
8010136c:  beq a1,v0,0x80101388
80101370:  _sw ra,0x14(sp)
80101374:  li v0,0x8
80101378:  beq a1,v0,0x8010139c
8010137c:  _clear v0
80101380:  slti a0,zero,0x4f4
80101384:  nop
80101388:  move a0,s0
8010138c:  jal 0x8002239c
80101390:  _move a1,a2
80101394:  slti a0,zero,0x4ea
80101398:  nop
8010139c:  move a0,s0
801013a0:  jal 0x80022320
801013a4:  _move a1,a2
801013a8:  beq v0,zero,0x801013d0
801013ac:  _clear v0
801013b0:  lh a0,0x6(s0)
801013b4:  jal 0x80021888
801013b8:  _addiu a0,a0,0x3e8
801013bc:  move s0,v0
801013c0:  beq s0,zero,0x801013d0
801013c4:  _nop
801013c8:  jal 0x800205f8
801013cc:  _move a0,s0
801013d0:  lw ra,0x14(sp)
801013d4:  lw s0,0x10(sp)
801013d8:  jr ra
801013dc:  _addiu sp,sp,0x18
