# addr: 0x8004b2c4  name: CdRead
8004b2c4:  addiu sp,sp,-0x18
8004b2c8:  move a3,a0
8004b2cc:  lui a0,0x8006
8004b2d0:  addiu a0,a0,0x398
8004b2d4:  sw ra,0x14(sp)
8004b2d8:  sw s0,0x10(sp)
8004b2dc:  sw a2,0xc(a0)
8004b2e0:  lw v0,0xc(a0)
8004b2e4:  nop
8004b2e8:  andi v1,v0,0x30
8004b2ec:  beq v1,zero,0x8004b304
8004b2f0:  _li v0,0x20
8004b2f4:  beq v1,v0,0x8004b310
8004b2f8:  _li v0,0x246
8004b2fc:  j 0x8004b31c
8004b300:  _nop
8004b304:  li v0,0x200
8004b308:  j 0x8004b328
8004b30c:  _sw v0,0x10(a0)
8004b310:  li v0,0x249
8004b314:  j 0x8004b328
8004b318:  _sw v0,0x10(a0)
