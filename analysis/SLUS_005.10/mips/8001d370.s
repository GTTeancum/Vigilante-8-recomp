# addr: 0x8001d370  name: FUN_8001d370
8001d370:  lw a1,0x4(gp)
8001d374:  addiu sp,sp,-0x18
8001d378:  lui a0,0x8007
8001d37c:  lui v0,0x8005
8001d380:  addiu v0,v0,0x693c
8001d384:  sw ra,0x10(sp)
8001d388:  sll a1,a1,0xe
8001d38c:  addu v1,a1,v0
8001d390:  addiu v0,v0,0x4000
8001d394:  addu a1,a1,v0
8001d398:  sw v1,-0x69c8(gp)
8001d39c:  sw a1,-0x69c4(gp)
8001d3a0:  jal 0x8004d3a4
8001d3a4:  _addiu a0,a0,-0x8a0
8001d3a8:  li a0,0x40
8001d3ac:  move a1,a0
8001d3b0:  jal 0x8004d4e4
8001d3b4:  _move a2,a0
8001d3b8:  lw a2,0x6d4(gp)
8001d3bc:  li a0,0x800
8001d3c0:  jal 0x8004c544
8001d3c4:  _li a1,0x2000
8001d3c8:  lw ra,0x10(sp)
8001d3cc:  nop
8001d3d0:  jr ra
8001d3d4:  _addiu sp,sp,0x18
