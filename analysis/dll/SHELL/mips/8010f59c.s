# addr: 0x8010f59c  name: FUN_8010f59c
8010f59c:  addiu sp,sp,-0x18
8010f5a0:  sw s0,0x10(sp)
8010f5a4:  lui s0,0x8011
8010f5a8:  addiu s0,s0,0x3448
8010f5ac:  sw ra,0x14(sp)
8010f5b0:  lw a0,0x0(s0)
8010f5b4:  nop
8010f5b8:  bltz a0,0x8010f5d0
8010f5bc:  _nop
8010f5c0:  sltiu a0,zero,0x4240
8010f5c4:  nop
8010f5c8:  li v0,-0x1
8010f5cc:  sw v0,0x0(s0)
8010f5d0:  lw ra,0x14(sp)
8010f5d4:  lw s0,0x10(sp)
8010f5d8:  jr ra
8010f5dc:  _addiu sp,sp,0x18
