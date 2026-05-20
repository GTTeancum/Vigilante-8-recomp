# addr: 0x8001f3ac  name: FUN_8001f3ac
8001f3ac:  addiu sp,sp,-0x58
8001f3b0:  sw s1,0x3c(sp)
8001f3b4:  move s1,a0
8001f3b8:  sw ra,0x50(sp)
8001f3bc:  sw s5,0x4c(sp)
8001f3c0:  sw s4,0x48(sp)
8001f3c4:  sw s3,0x44(sp)
8001f3c8:  sw s2,0x40(sp)
8001f3cc:  sw s0,0x38(sp)
8001f3d0:  lw s0,0x38(s1)
8001f3d4:  move s5,a1
8001f3d8:  move s3,a2
8001f3dc:  lw s2,0x68(sp)
8001f3e0:  beq s0,zero,0x8001f4f4
8001f3e4:  _move s4,a3
8001f3e8:  lw v0,0x5c(s0)
8001f3ec:  nop
8001f3f0:  beq v0,zero,0x8001f4a8
8001f3f4:  _move a0,s5
8001f3f8:  addiu a1,s0,0x10
8001f3fc:  jal 0x8004cf04
8001f400:  _addiu a2,sp,0x18
8001f404:  lh a0,0x20(sp)
8001f408:  nop
8001f40c:  bgtz a0,0x8001f46c
8001f410:  _nop
8001f414:  lh v1,0x1a(sp)
8001f418:  lh v0,0x12(s1)
8001f41c:  nop
8001f420:  mult v1,v0
8001f424:  lh v0,0x18(s1)
8001f428:  mflo a1
8001f42c:  nop
8001f430:  nop
8001f434:  mult a0,v0
8001f438:  lh v1,0x26(sp)
8001f43c:  lh v0,0x1e(s1)
8001f440:  mflo a0
8001f444:  nop
8001f448:  nop
8001f44c:  mult v1,v0
8001f450:  addu v0,a1,a0
8001f454:  mflo v1
8001f458:  addu v0,v0,v1
8001f45c:  slti v0,v0,0x801
8001f460:  xori v0,v0,0x1
8001f464:  beq v0,zero,0x8001f48c
8001f468:  _nop
8001f46c:  sw s2,0x10(sp)
8001f470:  lw a0,0x5c(s0)
8001f474:  addiu a1,sp,0x18
8001f478:  move a2,s3
8001f47c:  jal 0x8001ef74
8001f480:  _move a3,s4
8001f484:  bne v0,zero,0x8001f4f8
8001f488:  _nop
8001f48c:  lw v0,0x0(s0)
8001f490:  nop
8001f494:  andi v0,v0,0x800
8001f498:  beq v0,zero,0x8001f4e4
8001f49c:  _nop
8001f4a0:  j 0x8001f4c8
8001f4a4:  _sw s2,0x10(sp)
8001f4a8:  lw v0,0x0(s0)
8001f4ac:  nop
8001f4b0:  andi v0,v0,0x800
8001f4b4:  beq v0,zero,0x8001f4e4
8001f4b8:  _addiu a1,s0,0x10
8001f4bc:  jal 0x8004cf04
8001f4c0:  _addiu a2,sp,0x18
8001f4c4:  sw s2,0x10(sp)
8001f4c8:  move a0,s0
8001f4cc:  addiu a1,sp,0x18
8001f4d0:  move a2,s3
8001f4d4:  jal 0x8001f3ac
8001f4d8:  _move a3,s4
8001f4dc:  bne v0,zero,0x8001f4f8
8001f4e0:  _nop
8001f4e4:  lw s0,0x34(s0)
8001f4e8:  nop
8001f4ec:  bne s0,zero,0x8001f3e8
8001f4f0:  _nop
8001f4f4:  clear v0
8001f4f8:  lw ra,0x50(sp)
8001f4fc:  lw s5,0x4c(sp)
8001f500:  lw s4,0x48(sp)
8001f504:  lw s3,0x44(sp)
8001f508:  lw s2,0x40(sp)
8001f50c:  lw s1,0x3c(sp)
8001f510:  lw s0,0x38(sp)
8001f514:  jr ra
8001f518:  _addiu sp,sp,0x58
