# addr: 0x8010f178  name: FUN_8010f178
8010f178:  addiu sp,sp,-0x18
8010f17c:  lui v1,0x8011
8010f180:  addiu v1,v1,0x3438
8010f184:  sw ra,0x10(sp)
8010f188:  lw v0,0x0(v1)
8010f18c:  nop
8010f190:  bne v0,zero,0x8010f1c0
8010f194:  _move a1,a0
8010f198:  lui a0,0x8011
8010f19c:  addiu a0,a0,-0xe20
8010f1a0:  li v0,0x2
8010f1a4:  sw v0,0x0(v1)
8010f1a8:  sw zero,0x4(v1)
8010f1ac:  sw zero,0x8(v1)
8010f1b0:  sltiu a0,zero,0x4300
8010f1b4:  sw a1,0xc(v1)
8010f1b8:  slti a0,zero,0x3c74
8010f1bc:  li v0,0x1
8010f1c0:  lui a0,0x8010
8010f1c4:  jal 0x80052604
8010f1c8:  _addiu a0,a0,0x10e4
8010f1cc:  clear v0
8010f1d0:  lw ra,0x10(sp)
8010f1d4:  addiu sp,sp,0x18
8010f1d8:  jr ra
8010f1dc:  _nop
