# addr: 0x8003e2fc  name: FUN_8003e2fc
8003e2fc:  addiu sp,sp,-0x80
8003e300:  sw s3,0x74(sp)
8003e304:  move s3,a0
8003e308:  sw ra,0x78(sp)
8003e30c:  sw s2,0x70(sp)
8003e310:  sw s1,0x6c(sp)
8003e314:  sw s0,0x68(sp)
8003e318:  lw a0,0x24(s3)
8003e31c:  lw a1,0x2c(s3)
8003e320:  lw s1,0x70(s3)
8003e324:  jal 0x80025400
8003e328:  _nop
8003e32c:  lw v1,0x24(s3)
8003e330:  nop
8003e334:  sw v1,0x18(s1)
8003e338:  lw v1,0x2c(s3)
8003e33c:  nop
8003e340:  sw v1,0x20(s1)
8003e344:  lw a0,0x74(s3)
8003e348:  nop
8003e34c:  beq a0,zero,0x8003e39c
8003e350:  _move s0,v0
8003e354:  move a1,s0
8003e358:  addiu s2,s3,0x24
8003e35c:  move a2,s2
8003e360:  jal 0x8001f51c
8003e364:  _addiu a3,sp,0x10
8003e368:  bne v0,zero,0x8003e394
8003e36c:  _nop
8003e370:  lw a0,0x78(s3)
8003e374:  nop
8003e378:  beq a0,zero,0x8003e39c
8003e37c:  _move a1,s0
8003e380:  move a2,s2
8003e384:  jal 0x8001f51c
8003e388:  _addiu a3,sp,0x10
8003e38c:  beq v0,zero,0x8003e39c
8003e390:  _nop
8003e394:  j 0x8003e3b0
8003e398:  _sw v0,0x1c(s1)
8003e39c:  sw s0,0x1c(s1)
8003e3a0:  lw a0,0x24(s3)
8003e3a4:  lw a1,0x2c(s3)
8003e3a8:  jal 0x80025800
8003e3ac:  _addiu a2,sp,0x10
8003e3b0:  lw v0,0x0(s1)
8003e3b4:  nop
8003e3b8:  lhu v0,0x0(v0)
8003e3bc:  nop
8003e3c0:  andi v0,v0,0x8
8003e3c4:  beq v0,zero,0x8003e448
8003e3c8:  _addiu v1,sp,0x18
8003e3cc:  lw v0,0x24(s1)
8003e3d0:  nop
8003e3d4:  sw v0,0x18(sp)
8003e3d8:  sw zero,0x4(v1)
8003e3dc:  lw v0,0x28(s1)
8003e3e0:  nop
8003e3e4:  sw v0,0x8(v1)
8003e3e8:  li v0,0x1000
8003e3ec:  sh v0,0x4(s1)
8003e3f0:  sh v0,0x14(s1)
8003e3f4:  sh zero,0x12(s1)
8003e3f8:  sh zero,0x10(s1)
8003e3fc:  sh zero,0xc(s1)
8003e400:  sh zero,0x8(s1)
8003e404:  sh zero,0x6(s1)
8003e408:  lh v0,0x10(sp)
8003e40c:  lh v1,0x12(sp)
8003e410:  subu v0,zero,v0
8003e414:  sll v0,v0,0xc
8003e418:  div v0,v1
8003e41c:  mflo v0
8003e420:  nop
8003e424:  sh v0,0xa(s1)
8003e428:  lh v0,0x14(sp)
8003e42c:  lh v1,0x12(sp)
8003e430:  subu v0,zero,v0
8003e434:  sll v0,v0,0xc
8003e438:  div v0,v1
8003e43c:  mflo v0
8003e440:  j 0x8003e504
8003e444:  _sh v0,0xe(s1)
8003e448:  lh v1,0x10(sp)
8003e44c:  lh a0,0x12(sp)
8003e450:  li v0,0x1000
8003e454:  sh v0,0x28(sp)
8003e458:  sh v0,0x38(sp)
8003e45c:  sh zero,0x36(sp)
8003e460:  sh zero,0x34(sp)
8003e464:  sh zero,0x30(sp)
8003e468:  sh zero,0x2c(sp)
8003e46c:  sh zero,0x2a(sp)
8003e470:  subu v1,zero,v1
8003e474:  beq a0,zero,0x8003e48c
8003e478:  _sll v0,v1,0xc
8003e47c:  div v0,a0
8003e480:  mflo v0
8003e484:  j 0x8003e490
8003e488:  _move v1,v0
8003e48c:  sll v1,v1,0x4
8003e490:  lh v0,0x14(sp)
8003e494:  lh a0,0x12(sp)
8003e498:  sh v1,0x2e(sp)
8003e49c:  subu v1,zero,v0
8003e4a0:  beq a0,zero,0x8003e4b8
8003e4a4:  _sll v0,v1,0xc
8003e4a8:  div v0,a0
8003e4ac:  mflo v0
8003e4b0:  j 0x8003e4c0
8003e4b4:  _sh v0,0x32(sp)
8003e4b8:  sll v0,v1,0x4
8003e4bc:  sh v0,0x32(sp)
8003e4c0:  lh v0,0x18(s3)
8003e4c4:  lw v1,0x24(s1)
8003e4c8:  bgtz v0,0x8003e4d4
8003e4cc:  _addiu a1,sp,0x18
8003e4d0:  subu v1,zero,v1
8003e4d4:  sw v1,0x18(sp)
8003e4d8:  sw zero,0x4(a1)
8003e4dc:  lw v0,0x28(s1)
8003e4e0:  addiu a0,s3,0x10
8003e4e4:  addiu s0,sp,0x48
8003e4e8:  move a2,s0
8003e4ec:  jal 0x80043754
8003e4f0:  _sw v0,0x8(a1)
8003e4f4:  addiu a0,sp,0x28
8003e4f8:  move a1,s0
8003e4fc:  jal 0x8004ccb4
8003e500:  _addiu a2,s1,0x4
8003e504:  lw ra,0x78(sp)
8003e508:  lw s3,0x74(sp)
8003e50c:  lw s2,0x70(sp)
8003e510:  lw s1,0x6c(sp)
8003e514:  lw s0,0x68(sp)
8003e518:  jr ra
8003e51c:  _addiu sp,sp,0x80
