# addr: 0x801002c4  name: FUN_801002c4
801002c4:  addiu sp,sp,-0x18
801002c8:  sw ra,0x10(sp)
801002cc:  li v0,0x1
801002d0:  beq a1,v0,0x80100300
801002d4:  _move v1,a0
801002d8:  li v0,0x7
801002dc:  bne a1,v0,0x8010030c
801002e0:  _clear v0
801002e4:  lui v0,0x8006
801002e8:  lw v0,0x59fc(v0)
801002ec:  li a0,0x80
801002f0:  jal 0x8001d470
801002f4:  _sw v1,0x58(v0)
801002f8:  slti a0,zero,0xc3
801002fc:  nop
80100300:  jal 0x80045088
80100304:  _move a0,v1
80100308:  li v0,-0x1
8010030c:  lw ra,0x10(sp)
80100310:  nop
80100314:  jr ra
80100318:  _addiu sp,sp,0x18
