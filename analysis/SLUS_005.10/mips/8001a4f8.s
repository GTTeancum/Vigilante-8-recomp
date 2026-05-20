# addr: 0x8001a4f8  name: FUN_8001a4f8
8001a4f8:  addiu sp,sp,-0x18
8001a4fc:  sw s0,0x10(sp)
8001a500:  move s0,a0
8001a504:  sw ra,0x14(sp)
8001a508:  lw v0,0x10(s0)
8001a50c:  nop
8001a510:  beq v0,zero,0x8001a540
8001a514:  _nop
8001a518:  bne a1,zero,0x8001a53c
8001a51c:  _lui v0,0xff
8001a520:  ori v0,v0,0xffff
8001a524:  addiu v1,s0,0x48
8001a528:  and v1,v1,v0
8001a52c:  lui v0,0x500
8001a530:  or v1,v1,v0
8001a534:  j 0x8001a540
8001a538:  _sw v1,0x30(s0)
8001a53c:  sw zero,0x30(s0)
8001a540:  bne a1,zero,0x8001a55c
8001a544:  _addiu a0,s0,0x18
8001a548:  lw v0,0x10(s0)
8001a54c:  nop
8001a550:  bne v0,zero,0x8001a55c
8001a554:  _addiu a0,s0,0x30
8001a558:  addiu a0,s0,0x48
8001a55c:  jal 0x8004fb74
8001a560:  _nop
8001a564:  lw a0,0x8(s0)
8001a568:  lw a1,0xc(s0)
8001a56c:  jal 0x8004d524
8001a570:  _nop
8001a574:  lw ra,0x14(sp)
8001a578:  lw s0,0x10(sp)
8001a57c:  jr ra
8001a580:  _addiu sp,sp,0x18
