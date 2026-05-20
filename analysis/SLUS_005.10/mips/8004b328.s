# addr: 0x8004b328  name: CDREAD_OBJ_614
8004b328:  lui s0,0x8006
8004b32c:  addiu s0,s0,0x398
8004b330:  lw v0,0xc(s0)
8004b334:  clear a0
8004b338:  ori v0,v0,0x20
8004b33c:  sw v0,0xc(s0)
8004b340:  sw a1,0x4(s0)
8004b344:  jal 0x80048fa8
8004b348:  _sw a3,0x0(s0)
8004b34c:  clear a0
8004b350:  jal 0x80048fbc
8004b354:  _sw v0,0x24(s0)
8004b358:  sw v0,0x28(s0)
8004b35c:  lw v0,0x30(s0)
8004b360:  nop
8004b364:  andi v0,v0,0x1
8004b368:  beq v0,zero,0x8004b37c
8004b36c:  _nop
8004b370:  jal 0x800493ec
8004b374:  _clear a0
8004b378:  sw v0,0x2c(s0)
8004b37c:  jal 0x80047e44
8004b380:  _li a0,-0x1
8004b384:  sw v0,0x1c(s0)
8004b388:  jal 0x80048e24
8004b38c:  _nop
8004b390:  andi v0,v0,0xe0
8004b394:  beq v0,zero,0x8004b3a8
8004b398:  _li a0,0x9
8004b39c:  clear a1
8004b3a0:  jal 0x80049240
8004b3a4:  _clear a2
8004b3a8:  jal 0x8004b040
8004b3ac:  _clear a0
8004b3b0:  slt v0,zero,v0
8004b3b4:  lw ra,0x14(sp)
8004b3b8:  lw s0,0x10(sp)
8004b3bc:  jr ra
8004b3c0:  _addiu sp,sp,0x18
