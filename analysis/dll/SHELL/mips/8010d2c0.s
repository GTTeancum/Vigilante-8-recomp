# addr: 0x8010d2c0  name: FUN_8010d2c0
8010d2c0:  addiu sp,sp,-0x20
8010d2c4:  clear a0
8010d2c8:  sw ra,0x18(sp)
8010d2cc:  sw s1,0x14(sp)
8010d2d0:  sltiu a0,zero,0x38fb
8010d2d4:  sw s0,0x10(sp)
8010d2d8:  sltiu a0,zero,0x373c
8010d2dc:  lui s1,0x8011
8010d2e0:  li a0,0x9
8010d2e4:  clear a1
8010d2e8:  jal 0x80049240
8010d2ec:  _move a2,a1
8010d2f0:  addiu s0,s1,0x33c0
8010d2f4:  lw a0,0x4(s0)
8010d2f8:  jal 0x80045088
8010d2fc:  _nop
8010d300:  lw a0,0x33c0(s1)
8010d304:  jal 0x80045088
8010d308:  _nop
8010d30c:  lw a0,0x8(s0)
8010d310:  jal 0x80045088
8010d314:  _nop
8010d318:  lw a0,0xc(s0)
8010d31c:  jal 0x80045088
8010d320:  _nop
8010d324:  lw a0,0x1c(s0)
8010d328:  jal 0x80045088
8010d32c:  _nop
8010d330:  lw a0,0x20(s0)
8010d334:  jal 0x80045088
8010d338:  _nop
8010d33c:  lw ra,0x18(sp)
8010d340:  lw s1,0x14(sp)
8010d344:  lw s0,0x10(sp)
8010d348:  li v0,0x1
8010d34c:  jr ra
8010d350:  _addiu sp,sp,0x20
