# addr: 0x8003c288  name: FUN_8003c288
8003c288:  addiu sp,sp,-0x38
8003c28c:  sw ra,0x30(sp)
8003c290:  sw s3,0x2c(sp)
8003c294:  sw s2,0x28(sp)
8003c298:  sw s1,0x24(sp)
8003c29c:  sw s0,0x20(sp)
8003c2a0:  lw v1,0x4(a1)
8003c2a4:  move s2,a0
8003c2a8:  lw v0,0x48(s2)
8003c2ac:  addiu a1,a1,0x4
8003c2b0:  subu a3,v1,v0
8003c2b4:  sw a3,0x10(sp)
8003c2b8:  lw v0,0x4(a1)
8003c2bc:  addiu a0,s2,0x48
8003c2c0:  lw v1,0x4(a0)
8003c2c4:  addiu a2,sp,0x10
8003c2c8:  subu v0,v0,v1
8003c2cc:  sw v0,0x4(a2)
8003c2d0:  lw v0,0x8(a1)
8003c2d4:  lw v1,0x8(a0)
8003c2d8:  nop
8003c2dc:  subu v0,v0,v1
8003c2e0:  sw v0,0x8(a2)
8003c2e4:  lb v0,0x8(s2)
8003c2e8:  nop
8003c2ec:  bltz v0,0x8003c350
8003c2f0:  _nop
8003c2f4:  lw v0,0x14(sp)
8003c2f8:  bgez a3,0x8003c304
8003c2fc:  _move v1,a3
8003c300:  subu v1,zero,v1
8003c304:  bgez v0,0x8003c310
8003c308:  _move a0,v0
8003c30c:  subu a0,zero,a0
8003c310:  slt v0,a0,v1
8003c314:  beq v0,zero,0x8003c320
8003c318:  _nop
8003c31c:  move a0,v1
8003c320:  lw v0,0x18(sp)
8003c324:  nop
8003c328:  bgez v0,0x8003c334
8003c32c:  _move v1,v0
8003c330:  subu v1,zero,v1
8003c334:  slt v0,v1,a0
8003c338:  beq v0,zero,0x8003c348
8003c33c:  _slti v0,v1,0x801
8003c340:  move v1,a0
8003c344:  slti v0,v1,0x801
8003c348:  bne v0,zero,0x8003c51c
8003c34c:  _li v0,0x1
8003c350:  lw v0,0x10(sp)
8003c354:  nop
8003c358:  bgez v0,0x8003c364
8003c35c:  _nop
8003c360:  addiu v0,v0,0x1f
8003c364:  lw a0,0x18(sp)
8003c368:  nop
8003c36c:  bgez a0,0x8003c378
8003c370:  _sra a1,v0,0x5
8003c374:  addiu a0,a0,0x7
8003c378:  lw v1,0x48(s2)
8003c37c:  sra v0,a0,0x3
8003c380:  addu v0,a1,v0
8003c384:  addu v1,v1,v0
8003c388:  sw v1,0x48(s2)
8003c38c:  lw v1,0x14(sp)
8003c390:  nop
8003c394:  bgez v1,0x8003c3a0
8003c398:  _nop
8003c39c:  addiu v1,v1,0xf
8003c3a0:  lw v0,0x4c(s2)
8003c3a4:  sra v1,v1,0x4
8003c3a8:  addu v0,v0,v1
8003c3ac:  sw v0,0x4c(s2)
8003c3b0:  lw v0,0x18(sp)
8003c3b4:  nop
8003c3b8:  bgez v0,0x8003c3c4
8003c3bc:  _nop
8003c3c0:  addiu v0,v0,0x1f
8003c3c4:  lw a0,0x10(sp)
8003c3c8:  nop
8003c3cc:  bgez a0,0x8003c3d8
8003c3d0:  _sra a1,v0,0x5
8003c3d4:  addiu a0,a0,0x7
8003c3d8:  lw v1,0x50(s2)
8003c3dc:  sra v0,a0,0x3
8003c3e0:  subu v0,a1,v0
8003c3e4:  addu v1,v1,v0
8003c3e8:  sw v1,0x50(s2)
8003c3ec:  lw t0,0x48(s2)
8003c3f0:  lw t1,0x4c(s2)
8003c3f4:  lw t2,0x50(s2)
8003c3f8:  sw t0,0x24(s2)
8003c3fc:  sw t1,0x28(s2)
8003c400:  sw t2,0x2c(s2)
8003c404:  lb v0,0x8(s2)
8003c408:  nop
8003c40c:  bgez v0,0x8003c51c
8003c410:  _clear v0
8003c414:  lbu v1,0x9(s2)
8003c418:  lw v0,0xc(gp)
8003c41c:  nop
8003c420:  subu v0,v0,v1
8003c424:  andi v0,v0,0x3
8003c428:  bne v0,zero,0x8003c51c
8003c42c:  _clear v0
8003c430:  jal 0x8001d5a0
8003c434:  _move a0,s2
8003c438:  move s3,v0
8003c43c:  clear s1
8003c440:  li s0,0x24
8003c444:  addu v0,s3,s0
8003c448:  lw a0,0xec(v0)
8003c44c:  lb v0,0x8(s2)
8003c450:  lb v1,0x8(a0)
8003c454:  subu v0,zero,v0
8003c458:  bne v1,v0,0x8003c4ac
8003c45c:  _nop
8003c460:  lw v0,0x64(a0)
8003c464:  nop
8003c468:  beq v0,zero,0x8003c480
8003c46c:  _li a1,0xf
8003c470:  jalr v0
8003c474:  _move a2,s2
8003c478:  j 0x8003c484
8003c47c:  _nop
8003c480:  clear v0
8003c484:  bne v0,zero,0x8003c4ac
8003c488:  _addu v0,s3,s0
8003c48c:  lw a0,0xec(v0)
8003c490:  nop
8003c494:  lhu v1,0xc(a0)
8003c498:  nop
8003c49c:  sltiu v0,v1,0x63
8003c4a0:  beq v0,zero,0x8003c4ac
8003c4a4:  _addiu v0,v1,0x1
8003c4a8:  sh v0,0xc(a0)
8003c4ac:  addiu s1,s1,0x1
8003c4b0:  slti v0,s1,0x3
8003c4b4:  bne v0,zero,0x8003c444
8003c4b8:  _addiu s0,s0,0x4
8003c4bc:  lhu v0,0xc(s2)
8003c4c0:  nop
8003c4c4:  addiu v0,v0,-0x1
8003c4c8:  sh v0,0xc(s2)
8003c4cc:  andi v0,v0,0xffff
8003c4d0:  bne v0,zero,0x8003c518
8003c4d4:  _nop
8003c4d8:  jal 0x8004410c
8003c4dc:  _nop
8003c4e0:  move a0,s2
8003c4e4:  jal 0x8001d624
8003c4e8:  _move s0,v0
8003c4ec:  lw a1,0x5f8(gp)
8003c4f0:  move a0,s0
8003c4f4:  li a2,0x28
8003c4f8:  jal 0x8004483c
8003c4fc:  _addiu a3,v0,0x14
8003c500:  jal 0x8001d564
8003c504:  _move a0,s2
8003c508:  jal 0x800204dc
8003c50c:  _move a0,s2
8003c510:  j 0x8003c51c
8003c514:  _li v0,-0x1
8003c518:  clear v0
8003c51c:  lw ra,0x30(sp)
8003c520:  lw s3,0x2c(sp)
8003c524:  lw s2,0x28(sp)
8003c528:  lw s1,0x24(sp)
8003c52c:  lw s0,0x20(sp)
8003c530:  jr ra
8003c534:  _addiu sp,sp,0x38
