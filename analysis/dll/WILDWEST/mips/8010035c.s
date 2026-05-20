# addr: 0x8010035c  name: FUN_8010035c
8010035c:  addiu sp,sp,-0x18
80100360:  sw s0,0x10(sp)
80100364:  move s0,a0
80100368:  move v1,a1
8010036c:  move a1,a2
80100370:  li v0,0x3
80100374:  beq v1,v0,0x80100390
80100378:  _sw ra,0x14(sp)
8010037c:  li v0,0x8
80100380:  beq v1,v0,0x801003ac
80100384:  _clear v0
80100388:  slti a0,zero,0xf7
8010038c:  nop
80100390:  lw a1,0x0(a1)
80100394:  nop
80100398:  lbu v1,0x4(a1)
8010039c:  li v0,0x7
801003a0:  bne v1,v0,0x801003dc
801003a4:  _clear v0
801003a8:  lhu a1,0xc(a1)
801003ac:  jal 0x80022320
801003b0:  _move a0,s0
801003b4:  beq v0,zero,0x801003dc
801003b8:  _clear v0
801003bc:  lh a0,0x6(s0)
801003c0:  jal 0x8001fd9c
801003c4:  _nop
801003c8:  beq v0,zero,0x801003d8
801003cc:  _addiu a0,v0,0xc
801003d0:  jal 0x80024718
801003d4:  _ori a1,zero,0x8f80
801003d8:  li v0,-0x1
801003dc:  lw ra,0x14(sp)
801003e0:  lw s0,0x10(sp)
801003e4:  jr ra
801003e8:  _addiu sp,sp,0x18
