# addr: 0x8002d494  name: FUN_8002d494
8002d494:  addiu sp,sp,-0x30
8002d498:  sw s6,0x28(sp)
8002d49c:  move s6,a1
8002d4a0:  sw ra,0x2c(sp)
8002d4a4:  sw s5,0x24(sp)
8002d4a8:  sw s4,0x20(sp)
8002d4ac:  sw s3,0x1c(sp)
8002d4b0:  sw s2,0x18(sp)
8002d4b4:  sw s1,0x14(sp)
8002d4b8:  sw s0,0x10(sp)
8002d4bc:  lw s5,0x8(s6)
8002d4c0:  move s3,a0
8002d4c4:  lw a1,0x0(s3)
8002d4c8:  lui v1,0x2000
8002d4cc:  andi v0,s5,0x40
8002d4d0:  sltu v0,zero,v0
8002d4d4:  sll a0,v0,0x1d
8002d4d8:  and v0,a1,v1
8002d4dc:  beq v0,a0,0x8002d540
8002d4e0:  _addiu s4,s3,0xa4
8002d4e4:  xor v0,a1,v1
8002d4e8:  sw v0,0x0(s3)
8002d4ec:  lw v1,0xf8(s3)
8002d4f0:  beq a0,zero,0x8002d500
8002d4f4:  _li v0,0x800
8002d4f8:  j 0x8002d504
8002d4fc:  _sh v0,0x42(v1)
8002d500:  sh zero,0x42(v1)
8002d504:  lw v1,0xe0(s3)
8002d508:  beq a0,zero,0x8002d518
8002d50c:  _li v0,0x800
8002d510:  j 0x8002d51c
8002d514:  _sh v0,0x8e(v1)
8002d518:  sh zero,0x8e(v1)
8002d51c:  lw v1,0xe0(s3)
8002d520:  nop
8002d524:  lw v0,0x98(v1)
8002d528:  nop
8002d52c:  subu v0,zero,v0
8002d530:  sw v0,0x98(v1)
8002d534:  lw a0,0xf8(s3)
8002d538:  jal 0x8001d708
8002d53c:  _nop
8002d540:  lui v0,0x18
8002d544:  and v0,s5,v0
8002d548:  beq v0,zero,0x8002d5d8
8002d54c:  _lui v0,0x8
8002d550:  and v0,s5,v0
8002d554:  beq v0,zero,0x8002d560
8002d558:  _li a1,-0x1
8002d55c:  li a1,0x1
8002d560:  jal 0x8002cf90
8002d564:  _move a0,s3
8002d568:  lw v1,0x0(s3)
8002d56c:  lui a0,0xefff
8002d570:  ori a0,a0,0xffff
8002d574:  move s0,v0
8002d578:  and v1,v1,a0
8002d57c:  jal 0x8004410c
8002d580:  _sw v1,0x0(s3)
8002d584:  lw a1,0x5f8(gp)
8002d588:  beq s0,zero,0x8002d594
8002d58c:  _li a2,0x15
8002d590:  li a2,0x16
8002d594:  jal 0x8004445c
8002d598:  _move a0,v0
8002d59c:  beq s0,zero,0x8002d5d8
8002d5a0:  _nop
8002d5a4:  lbu v0,0xf(s4)
8002d5a8:  nop
8002d5ac:  addiu v0,v0,0x9
8002d5b0:  sll v0,v0,0x2
8002d5b4:  addu v0,s3,v0
8002d5b8:  lw a0,0xec(v0)
8002d5bc:  nop
8002d5c0:  lw v0,0x64(a0)
8002d5c4:  nop
8002d5c8:  beq v0,zero,0x8002d5d8
8002d5cc:  _li a1,0xa
8002d5d0:  jalr v0
8002d5d4:  _move a2,s3
8002d5d8:  lw v0,0x0(s3)
8002d5dc:  lui v1,0x1000
8002d5e0:  and v0,v0,v1
8002d5e4:  bne v0,zero,0x8002d618
8002d5e8:  _lui v0,0x20
8002d5ec:  lbu v1,0x9(s3)
8002d5f0:  lw v0,0xc(gp)
8002d5f4:  nop
8002d5f8:  subu v0,v0,v1
8002d5fc:  andi v0,v0,0x3f
8002d600:  bne v0,zero,0x8002d618
8002d604:  _lui v0,0x20
8002d608:  move a0,s3
8002d60c:  jal 0x8002ea94
8002d610:  _clear a1
8002d614:  lui v0,0x20
8002d618:  and v0,s5,v0
8002d61c:  beq v0,zero,0x8002d684
8002d620:  _lui v1,0x1000
8002d624:  lw v0,0x0(s3)
8002d628:  nop
8002d62c:  and v0,v0,v1
8002d630:  beq v0,zero,0x8002d650
8002d634:  _clear a1
8002d638:  lh v0,0x18(s4)
8002d63c:  nop
8002d640:  slti v0,v0,0x100
8002d644:  beq v0,zero,0x8002d650
8002d648:  _nop
8002d64c:  lw a1,0xe4(s3)
8002d650:  jal 0x8002ed34
8002d654:  _move a0,s3
8002d658:  sw v0,0xe4(s3)
8002d65c:  sh zero,0x18(s4)
8002d660:  lw v0,0x0(s3)
8002d664:  lui v1,0x1000
8002d668:  or v0,v0,v1
8002d66c:  jal 0x8004410c
8002d670:  _sw v0,0x0(s3)
8002d674:  lw a1,0x5f8(gp)
8002d678:  move a0,v0
8002d67c:  jal 0x8004445c
8002d680:  _li a2,0x16
8002d684:  lui v0,0x4
8002d688:  and v0,s5,v0
8002d68c:  beq v0,zero,0x8002d6e0
8002d690:  _move a0,s3
8002d694:  lbu v0,0xf(s4)
8002d698:  nop
8002d69c:  sll v0,v0,0x2
8002d6a0:  addu v0,s3,v0
8002d6a4:  lw v0,0x110(v0)
8002d6a8:  nop
8002d6ac:  beq v0,zero,0x8002d6c4
8002d6b0:  _nop
8002d6b4:  lh v0,0x6(v0)
8002d6b8:  nop
8002d6bc:  beq v0,zero,0x8002d6e0
8002d6c0:  _nop
8002d6c4:  jal 0x8004410c
8002d6c8:  _nop
8002d6cc:  lw a1,0x5f8(gp)
8002d6d0:  move a0,v0
8002d6d4:  jal 0x8004445c
8002d6d8:  _li a2,0x15
8002d6dc:  move a0,s3
8002d6e0:  jal 0x8002ce68
8002d6e4:  _andi a1,s5,0x4
8002d6e8:  lhu v0,0xa(s4)
8002d6ec:  nop
8002d6f0:  beq v0,zero,0x8002d700
8002d6f4:  _addiu v0,v0,-0x1
8002d6f8:  j 0x8002d7bc
8002d6fc:  _sh v0,0xa(s4)
8002d700:  lui v0,0x2
8002d704:  and v0,s5,v0
8002d708:  beq v0,zero,0x8002d7c0
8002d70c:  _andi v0,s5,0x2
8002d710:  lw v0,0x4(s6)
8002d714:  nop
8002d718:  slti v0,v0,0x101
8002d71c:  bne v0,zero,0x8002d7c0
8002d720:  _andi v0,s5,0x2
8002d724:  clear s2
8002d728:  li s1,0x24
8002d72c:  addu v0,s3,s1
8002d730:  lw a0,0xec(v0)
8002d734:  nop
8002d738:  beq a0,zero,0x8002d7ac
8002d73c:  _nop
8002d740:  lhu v0,0xc(a0)
8002d744:  nop
8002d748:  beq v0,zero,0x8002d7ac
8002d74c:  _nop
8002d750:  lw v0,0x64(a0)
8002d754:  lw a2,0x4(s6)
8002d758:  beq v0,zero,0x8002d76c
8002d75c:  _clear s0
8002d760:  jalr v0
8002d764:  _li a1,0x9
8002d768:  move s0,v0
8002d76c:  beq s0,zero,0x8002d7ac
8002d770:  _nop
8002d774:  jal 0x8004410c
8002d778:  _nop
8002d77c:  lw a1,0x5f8(gp)
8002d780:  bgez s0,0x8002d78c
8002d784:  _li a2,0x2a
8002d788:  li a2,0x15
8002d78c:  move a0,v0
8002d790:  jal 0x800447e8
8002d794:  _addiu a3,s3,0x24
8002d798:  blez s0,0x8002d7a4
8002d79c:  _nop
8002d7a0:  sh s0,0xa(s4)
8002d7a4:  j 0x8002d7bc
8002d7a8:  _sw zero,0x4(s6)
8002d7ac:  addiu s2,s2,0x1
8002d7b0:  slti v0,s2,0x3
8002d7b4:  bne v0,zero,0x8002d72c
8002d7b8:  _addiu s1,s1,0x4
8002d7bc:  andi v0,s5,0x2
8002d7c0:  beq v0,zero,0x8002d7cc
8002d7c4:  _li a1,0x4
8002d7c8:  li a1,0xb
8002d7cc:  lw a0,0x10c(s3)
8002d7d0:  nop
8002d7d4:  lw v0,0x64(a0)
8002d7d8:  nop
8002d7dc:  beq v0,zero,0x8002d7ec
8002d7e0:  _nop
8002d7e4:  jalr v0
8002d7e8:  _move a2,s3
8002d7ec:  lh v0,0x18(s4)
8002d7f0:  lhu v1,0x18(s4)
8002d7f4:  slti v0,v0,0x100
8002d7f8:  beq v0,zero,0x8002d804
8002d7fc:  _addiu v0,v1,0x8
8002d800:  sh v0,0x18(s4)
8002d804:  lw ra,0x2c(sp)
8002d808:  lw s6,0x28(sp)
8002d80c:  lw s5,0x24(sp)
8002d810:  lw s4,0x20(sp)
8002d814:  lw s3,0x1c(sp)
8002d818:  lw s2,0x18(sp)
8002d81c:  lw s1,0x14(sp)
8002d820:  lw s0,0x10(sp)
8002d824:  jr ra
8002d828:  _addiu sp,sp,0x30
