# addr: 0x8010d354  name: FUN_8010d354
8010d354:  addiu sp,sp,-0x50
8010d358:  sw s0,0x38(sp)
8010d35c:  move s0,a1
8010d360:  sw s2,0x40(sp)
8010d364:  move s2,a2
8010d368:  clear a2
8010d36c:  move a3,a2
8010d370:  li v1,0xf0
8010d374:  li v0,0x140
8010d378:  sw ra,0x4c(sp)
8010d37c:  sw s4,0x48(sp)
8010d380:  sw s3,0x44(sp)
8010d384:  sw s1,0x3c(sp)
8010d388:  sw zero,0x10(sp)
8010d38c:  sw v1,0x14(sp)
8010d390:  sw v0,0x18(sp)
8010d394:  sltiu a0,zero,0x340d
8010d398:  sw v1,0x1c(sp)
8010d39c:  beq v0,zero,0x8010d47c
8010d3a0:  _clear v0
8010d3a4:  jal 0x80017d5c
8010d3a8:  _lui s1,0x8006
8010d3ac:  addiu s4,s1,0x5930
8010d3b0:  lui v0,0x8011
8010d3b4:  addiu s3,v0,0x33c0
8010d3b8:  beq s2,zero,0x8010d3e4
8010d3bc:  _nop
8010d3c0:  jal 0x800120d4
8010d3c4:  _nop
8010d3c8:  lw v0,0x5930(s1)
8010d3cc:  lw v1,0x4(s4)
8010d3d0:  nop
8010d3d4:  or v0,v0,v1
8010d3d8:  and v0,v0,s2
8010d3dc:  bne v0,zero,0x8010d470
8010d3e0:  _nop
8010d3e4:  jal 0x80047e44
8010d3e8:  _clear a0
8010d3ec:  lw v1,0x38(s3)
8010d3f0:  li a3,0x140
8010d3f4:  sll v0,v1,0x4
8010d3f8:  subu v0,v0,v1
8010d3fc:  beq s0,zero,0x8010d408
8010d400:  _sll a2,v0,0x4
8010d404:  li a3,0x1e0
8010d408:  addiu a0,sp,0x20
8010d40c:  clear a1
8010d410:  li v0,0xf0
8010d414:  jal 0x8004f198
8010d418:  _sw v0,0x10(sp)
8010d41c:  lui v0,0x8006
8010d420:  lbu a1,0x531c(v0)
8010d424:  lui v0,0x8006
8010d428:  lbu v1,0x531d(v0)
8010d42c:  addiu a0,sp,0x20
8010d430:  li v0,0x140
8010d434:  sb s0,0x31(sp)
8010d438:  sh v0,0x24(sp)
8010d43c:  sll a1,a1,0x18
8010d440:  sra a1,a1,0x18
8010d444:  sll v1,v1,0x18
8010d448:  sra v1,v1,0x18
8010d44c:  sh a1,0x28(sp)
8010d450:  jal 0x8004fdb0
8010d454:  _sh v1,0x2a(sp)
8010d458:  jal 0x8004f4e8
8010d45c:  _li a0,0x1
8010d460:  sltiu a0,zero,0x3472
8010d464:  nop
8010d468:  bgez v0,0x8010d3b8
8010d46c:  _nop
8010d470:  sltiu a0,zero,0x34b0
8010d474:  nop
8010d478:  li v0,0x1
8010d47c:  lw ra,0x4c(sp)
8010d480:  lw s4,0x48(sp)
8010d484:  lw s3,0x44(sp)
8010d488:  lw s2,0x40(sp)
8010d48c:  lw s1,0x3c(sp)
8010d490:  lw s0,0x38(sp)
8010d494:  jr ra
8010d498:  _addiu sp,sp,0x50
