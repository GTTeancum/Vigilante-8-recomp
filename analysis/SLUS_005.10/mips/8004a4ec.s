# addr: 0x8004a4ec  name: CD_flush
8004a4ec:  lui v1,0x8006
8004a4f0:  lw v1,0x344(v1)
8004a4f4:  li v0,0x1
8004a4f8:  sb v0,0x0(v1)
8004a4fc:  lui v0,0x8006
8004a500:  lw v0,0x350(v0)
8004a504:  nop
8004a508:  lbu v0,0x0(v0)
8004a50c:  nop
8004a510:  andi v0,v0,0x7
8004a514:  beq v0,zero,0x8004a570
8004a518:  _li a0,0x1
8004a51c:  li v1,0x7
8004a520:  lui v0,0x8006
8004a524:  lw v0,0x344(v0)
8004a528:  nop
8004a52c:  sb a0,0x0(v0)
8004a530:  lui v0,0x8006
8004a534:  lw v0,0x350(v0)
8004a538:  nop
8004a53c:  sb v1,0x0(v0)
8004a540:  lui v0,0x8006
8004a544:  lw v0,0x34c(v0)
8004a548:  nop
8004a54c:  sb v1,0x0(v0)
8004a550:  lui v0,0x8006
8004a554:  lw v0,0x350(v0)
8004a558:  nop
8004a55c:  lbu v0,0x0(v0)
8004a560:  nop
8004a564:  andi v0,v0,0x7
8004a568:  bne v0,zero,0x8004a520
8004a56c:  _nop
8004a570:  lui v1,0x8006
8004a574:  addiu v1,v1,0x35c
8004a578:  sb zero,0x2(v1)
8004a57c:  lbu v0,0x2(v1)
8004a580:  nop
8004a584:  sb v0,0x1(v1)
8004a588:  lui a0,0x8006
8004a58c:  lw a0,0x344(a0)
8004a590:  li v0,0x2
8004a594:  sb v0,0x0(v1)
8004a598:  sb zero,0x0(a0)
8004a59c:  lui v0,0x8006
8004a5a0:  lw v0,0x350(v0)
8004a5a4:  nop
8004a5a8:  sb zero,0x0(v0)
8004a5ac:  lui v1,0x8006
8004a5b0:  lw v1,0x354(v1)
8004a5b4:  li v0,0x1325
8004a5b8:  jr ra
8004a5bc:  _sw v0,0x0(v1)
