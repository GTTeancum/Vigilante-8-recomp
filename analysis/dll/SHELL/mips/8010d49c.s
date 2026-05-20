# addr: 0x8010d49c  name: FUN_8010d49c
8010d49c:  addiu sp,sp,-0x30
8010d4a0:  sw s1,0x24(sp)
8010d4a4:  move s1,a2
8010d4a8:  li v0,0x280
8010d4ac:  sw v0,0x18(sp)
8010d4b0:  li v0,0x1e0
8010d4b4:  clear a2
8010d4b8:  move a3,a2
8010d4bc:  sw ra,0x2c(sp)
8010d4c0:  sw s2,0x28(sp)
8010d4c4:  sw s0,0x20(sp)
8010d4c8:  sw zero,0x10(sp)
8010d4cc:  sw zero,0x14(sp)
8010d4d0:  sltiu a0,zero,0x340d
8010d4d4:  sw v0,0x1c(sp)
8010d4d8:  bne v0,zero,0x8010d4e8
8010d4dc:  _lui s0,0x8006
8010d4e0:  slti a0,zero,0x3551
8010d4e4:  clear v0
8010d4e8:  addiu s2,s0,0x5930
8010d4ec:  beq s1,zero,0x8010d518
8010d4f0:  _nop
8010d4f4:  jal 0x800120d4
8010d4f8:  _nop
8010d4fc:  lw v0,0x5930(s0)
8010d500:  lw v1,0x4(s2)
8010d504:  nop
8010d508:  or v0,v0,v1
8010d50c:  and v0,v0,s1
8010d510:  bne v0,zero,0x8010d538
8010d514:  _nop
8010d518:  jal 0x80047e44
8010d51c:  _clear a0
8010d520:  jal 0x8004f4e8
8010d524:  _li a0,0x1
8010d528:  sltiu a0,zero,0x3472
8010d52c:  nop
8010d530:  bgez v0,0x8010d4ec
8010d534:  _nop
8010d538:  sltiu a0,zero,0x34b0
8010d53c:  nop
8010d540:  li v0,0x1
8010d544:  lw ra,0x2c(sp)
8010d548:  lw s2,0x28(sp)
8010d54c:  lw s1,0x24(sp)
8010d550:  lw s0,0x20(sp)
8010d554:  jr ra
8010d558:  _addiu sp,sp,0x30
