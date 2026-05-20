# addr: 0x8004e42c  name: RCpolyG3A
8004e42c:  lw t0,0x48(a3)
8004e430:  lw t1,0x4c(a3)
8004e434:  lw t2,0x50(a3)
8004e438:  gte_stH t9
8004e43c:  lw t4,0x14(t0)
8004e440:  lw t5,0x14(t1)
8004e444:  lw t6,0x14(t2)
8004e448:  sra t8,t9,0x1
8004e44c:  sltu at,t4,t8
8004e450:  beq at,zero,0x8004e474
8004e454:  _sltu at,t5,t8
8004e458:  beq at,zero,0x8004e474
8004e45c:  _sltu at,t6,t8
8004e460:  beq at,zero,0x8004e474
8004e464:  _nop
8004e468:  move v0,a0
8004e46c:  jr ra
8004e470:  _nop
8004e474:  gte_stOFX t9
8004e478:  lw v0,0x4(a1)
8004e47c:  lw v1,0x8(a1)
8004e480:  sra t9,t9,0x10
8004e484:  srl v0,v0,0x1
8004e488:  srl v1,v1,0x1
8004e48c:  addu t8,t9,v0
8004e490:  lh t4,0x10(t0)
8004e494:  lh t5,0x10(t1)
8004e498:  lh t6,0x10(t2)
8004e49c:  slt at,t8,t4
8004e4a0:  beq at,zero,0x8004e4c4
8004e4a4:  _slt at,t8,t5
8004e4a8:  beq at,zero,0x8004e4c4
8004e4ac:  _slt at,t8,t6
8004e4b0:  beq at,zero,0x8004e4c4
8004e4b4:  _nop
8004e4b8:  move v0,a0
8004e4bc:  jr ra
8004e4c0:  _nop
8004e4c4:  subu t8,t9,v0
8004e4c8:  slt at,t4,t8
8004e4cc:  beq at,zero,0x8004e4f0
8004e4d0:  _slt at,t5,t8
8004e4d4:  beq at,zero,0x8004e4f0
8004e4d8:  _slt at,t6,t8
8004e4dc:  beq at,zero,0x8004e4f0
8004e4e0:  _nop
8004e4e4:  move v0,a0
8004e4e8:  jr ra
8004e4ec:  _nop
8004e4f0:  gte_stOFY t9
8004e4f4:  lh t4,0x12(t0)
8004e4f8:  lh t5,0x12(t1)
8004e4fc:  lh t6,0x12(t2)
8004e500:  sra t9,t9,0x10
8004e504:  addu t8,t9,v1
8004e508:  slt at,t8,t4
8004e50c:  beq at,zero,0x8004e530
8004e510:  _slt at,t8,t5
8004e514:  beq at,zero,0x8004e530
8004e518:  _slt at,t8,t6
8004e51c:  beq at,zero,0x8004e530
8004e520:  _nop
8004e524:  move v0,a0
8004e528:  jr ra
8004e52c:  _nop
8004e530:  subu t8,t9,v1
8004e534:  slt at,t4,t8
8004e538:  beq at,zero,0x8004e55c
8004e53c:  _slt at,t5,t8
8004e540:  beq at,zero,0x8004e55c
8004e544:  _slt at,t6,t8
8004e548:  beq at,zero,0x8004e55c
8004e54c:  _nop
8004e550:  move v0,a0
8004e554:  jr ra
8004e558:  _nop
8004e55c:  lh t4,0x0(t0)
8004e560:  lh t5,0x0(t1)
8004e564:  lh t6,0x0(t2)
8004e568:  add t7,t4,t5
8004e56c:  add t8,t5,t6
8004e570:  add t9,t6,t4
8004e574:  sra t7,t7,0x1
8004e578:  sra t8,t8,0x1
8004e57c:  sra t9,t9,0x1
8004e580:  sh t7,0x0(a3)
8004e584:  sh t8,0x18(a3)
8004e588:  sh t9,0x30(a3)
8004e58c:  lh t4,0x2(t0)
8004e590:  lh t5,0x2(t1)
8004e594:  lh t6,0x2(t2)
8004e598:  add t7,t4,t5
8004e59c:  add t8,t5,t6
8004e5a0:  add t9,t6,t4
8004e5a4:  sra t7,t7,0x1
8004e5a8:  sra t8,t8,0x1
8004e5ac:  sra t9,t9,0x1
8004e5b0:  sh t7,0x2(a3)
8004e5b4:  sh t8,0x1a(a3)
8004e5b8:  sh t9,0x32(a3)
8004e5bc:  lh t4,0x4(t0)
8004e5c0:  lh t5,0x4(t1)
8004e5c4:  lh t6,0x4(t2)
8004e5c8:  add t7,t4,t5
8004e5cc:  add t8,t5,t6
8004e5d0:  add t9,t6,t4
8004e5d4:  sra t7,t7,0x1
8004e5d8:  sra t8,t8,0x1
8004e5dc:  sra t9,t9,0x1
8004e5e0:  sh t7,0x4(a3)
8004e5e4:  sh t8,0x1c(a3)
8004e5e8:  sh t9,0x34(a3)
8004e5ec:  ldv0 a3
8004e5f4:  gte_ldVXY1 0x18(a3)
8004e5f8:  gte_ldVZ1 0x1c(a3)
8004e5fc:  gte_ldVXY2 0x30(a3)
8004e600:  gte_ldVZ2 0x34(a3)
8004e604:  lbu t4,0xc(t0)
8004e608:  lbu t5,0xc(t1)
8004e60c:  lbu t6,0xc(t2)
8004e610:  RTPT
8004e614:  addu t7,t4,t5
8004e618:  addu t8,t5,t6
8004e61c:  addu t9,t6,t4
8004e620:  srl t7,t7,0x1
8004e624:  srl t8,t8,0x1
8004e628:  srl t9,t9,0x1
8004e62c:  sb t7,0xc(a3)
8004e630:  sb t8,0x24(a3)
8004e634:  sb t9,0x3c(a3)
8004e638:  lbu t4,0xd(t0)
8004e63c:  lbu t5,0xd(t1)
8004e640:  lbu t6,0xd(t2)
8004e644:  addu t7,t4,t5
8004e648:  addu t8,t5,t6
8004e64c:  addu t9,t6,t4
8004e650:  srl t7,t7,0x1
8004e654:  srl t8,t8,0x1
8004e658:  srl t9,t9,0x1
8004e65c:  sb t7,0xd(a3)
8004e660:  sb t8,0x25(a3)
8004e664:  sb t9,0x3d(a3)
8004e668:  lbu t4,0xe(t0)
8004e66c:  lbu t5,0xe(t1)
8004e670:  lbu t6,0xe(t2)
8004e674:  addu t7,t4,t5
8004e678:  addu t8,t5,t6
8004e67c:  addu t9,t6,t4
8004e680:  srl t7,t7,0x1
8004e684:  srl t8,t8,0x1
8004e688:  srl t9,t9,0x1
8004e68c:  sb t7,0xe(a3)
8004e690:  sb t8,0x26(a3)
8004e694:  sb t9,0x3e(a3)
8004e698:  lw t4,0x0(a1)
8004e69c:  addiu a2,a2,0x1
8004e6a0:  bne t4,a2,0x8004e718
8004e6a4:  _nop
8004e6a8:  gte_stSXY0 0x10(a3)
8004e6ac:  gte_stSXY1 0x28(a3)
8004e6b0:  gte_stSXY2 0x40(a3)
8004e6b4:  move v1,ra
8004e6b8:  lw t0,0x4c(a3)
8004e6bc:  addiu t1,a3,0x18
8004e6c0:  addiu t2,a3,0x0
8004e6c4:  jal 0x8004e7d0
8004e6c8:  _nop
8004e6cc:  addiu t0,a3,0x0
8004e6d0:  addiu t1,a3,0x18
8004e6d4:  addiu t2,a3,0x30
8004e6d8:  jal 0x8004e7d0
8004e6dc:  _nop
8004e6e0:  lw t0,0x48(a3)
8004e6e4:  addiu t1,a3,0x0
8004e6e8:  addiu t2,a3,0x30
8004e6ec:  jal 0x8004e7d0
8004e6f0:  _nop
8004e6f4:  lw t0,0x50(a3)
8004e6f8:  addiu t1,a3,0x30
8004e6fc:  addiu t2,a3,0x18
8004e700:  jal 0x8004e7d0
8004e704:  _nop
8004e708:  move ra,v1
8004e70c:  addiu a2,a2,-0x1
8004e710:  b 0x8004e7c4
8004e714:  _nop
8004e718:  gte_stSZ1 0x14(a3)
8004e71c:  gte_stSZ2 0x2c(a3)
8004e720:  gte_stSZ3 0x44(a3)
8004e724:  gte_stSXY0 0x10(a3)
8004e728:  gte_stSXY1 0x28(a3)
8004e72c:  gte_stSXY2 0x40(a3)
8004e730:  addiu a3,a3,0x58
8004e734:  sw ra,0x54(a3)
8004e738:  lw t4,-0x10(a3)
8004e73c:  addiu t5,a3,-0x58
8004e740:  addiu t6,a3,-0x28
8004e744:  sw t4,0x48(a3)
8004e748:  sw t5,0x4c(a3)
8004e74c:  sw t6,0x50(a3)
8004e750:  jal 0x8004e42c
8004e754:  _nop
8004e758:  lw t4,-0xc(a3)
8004e75c:  addiu t5,a3,-0x40
8004e760:  addiu t6,a3,-0x58
8004e764:  sw t4,0x48(a3)
8004e768:  sw t5,0x4c(a3)
8004e76c:  sw t6,0x50(a3)
8004e770:  jal 0x8004e42c
8004e774:  _nop
8004e778:  lw t4,-0x8(a3)
8004e77c:  addiu t5,a3,-0x28
8004e780:  addiu t6,a3,-0x40
8004e784:  sw t4,0x48(a3)
8004e788:  sw t5,0x4c(a3)
8004e78c:  sw t6,0x50(a3)
8004e790:  jal 0x8004e42c
8004e794:  _nop
8004e798:  addiu t4,a3,-0x58
8004e79c:  addiu t5,a3,-0x40
8004e7a0:  addiu t6,a3,-0x28
8004e7a4:  sw t4,0x48(a3)
8004e7a8:  sw t5,0x4c(a3)
8004e7ac:  sw t6,0x50(a3)
8004e7b0:  jal 0x8004e42c
8004e7b4:  _nop
8004e7b8:  lw ra,0x54(a3)
8004e7bc:  addiu a3,a3,-0x58
8004e7c0:  addiu a2,a2,-0x1
8004e7c4:  move v0,a0
8004e7c8:  jr ra
8004e7cc:  _nop
