# addr: 0x8002c59c  name: FUN_8002c59c
8002c59c:  addiu sp,sp,-0x20
8002c5a0:  sw s1,0x14(sp)
8002c5a4:  move s1,a0
8002c5a8:  sw s0,0x10(sp)
8002c5ac:  addiu s0,s1,0xa4
8002c5b0:  andi v0,a1,0xffff
8002c5b4:  beq v0,zero,0x8002c6dc
8002c5b8:  _sw ra,0x18(sp)
8002c5bc:  lhu v0,0xc(s0)
8002c5c0:  ori v1,zero,0xffff
8002c5c4:  sh zero,0x2(s0)
8002c5c8:  addiu v0,v0,-0x1
8002c5cc:  sh v0,0xc(s0)
8002c5d0:  andi v0,v0,0xffff
8002c5d4:  beq v0,v1,0x8002c648
8002c5d8:  _lui v0,0xffff
8002c5dc:  and v0,a1,v0
8002c5e0:  beq v0,zero,0x8002c6e8
8002c5e4:  _nop
8002c5e8:  jal 0x80017160
8002c5ec:  _nop
8002c5f0:  sll v1,v0,0x2
8002c5f4:  addu v1,v1,v0
8002c5f8:  sra v1,v1,0xf
8002c5fc:  beq v1,zero,0x8002c608
8002c600:  _li a0,0x13
8002c604:  li a0,0x12c
8002c608:  sh a0,0xc(s0)
8002c60c:  lb a0,0x5(s1)
8002c610:  nop
8002c614:  bne a0,zero,0x8002c630
8002c618:  _nop
8002c61c:  jal 0x8004410c
8002c620:  _nop
8002c624:  sb v0,0x5(s1)
8002c628:  sll v0,v0,0x18
8002c62c:  sra a0,v0,0x18
8002c630:  lw a1,0x5f8(gp)
8002c634:  li a2,0x21
8002c638:  jal 0x800443c8
8002c63c:  _clear a3
8002c640:  j 0x8002c6e8
8002c644:  _nop
8002c648:  lw v0,0x0(s1)
8002c64c:  lb a0,0x5(s1)
8002c650:  lui v1,0xf7ff
8002c654:  ori v1,v1,0xffff
8002c658:  and v0,v0,v1
8002c65c:  beq a0,zero,0x8002c66c
8002c660:  _sw v0,0x0(s1)
8002c664:  j 0x8002c680
8002c668:  _move a2,a0
8002c66c:  jal 0x8004410c
8002c670:  _nop
8002c674:  sb v0,0x5(s1)
8002c678:  sll v0,v0,0x18
8002c67c:  sra a2,v0,0x18
8002c680:  lh a0,0x6(s1)
8002c684:  lui v1,0x8007
8002c688:  addiu v1,v1,0x37a0
8002c68c:  li v0,0xf
8002c690:  subu v0,v0,a0
8002c694:  sll v0,v0,0x2
8002c698:  addu v0,v0,v1
8002c69c:  lw v0,0x0(v0)
8002c6a0:  nop
8002c6a4:  lw a1,0x8(v0)
8002c6a8:  move a0,a2
8002c6ac:  clear a2
8002c6b0:  jal 0x800443c8
8002c6b4:  _move a3,a2
8002c6b8:  jal 0x8004410c
8002c6bc:  _nop
8002c6c0:  lw a1,0x5f8(gp)
8002c6c4:  move a0,v0
8002c6c8:  li a2,0x1f
8002c6cc:  jal 0x800447e8
8002c6d0:  _addiu a3,s1,0x24
8002c6d4:  j 0x8002c6e8
8002c6d8:  _nop
8002c6dc:  lb a0,0x5(s1)
8002c6e0:  jal 0x800441c8
8002c6e4:  _nop
8002c6e8:  lw ra,0x18(sp)
8002c6ec:  lw s1,0x14(sp)
8002c6f0:  lw s0,0x10(sp)
8002c6f4:  jr ra
8002c6f8:  _addiu sp,sp,0x20
