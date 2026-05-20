# addr: 0x8010d55c  name: FUN_8010d55c
8010d55c:  lui v0,0x8011
8010d560:  lw v0,0x340c(v0)
8010d564:  addiu sp,sp,-0x20
8010d568:  sw ra,0x1c(sp)
8010d56c:  beq v0,zero,0x8010d594
8010d570:  _sw s0,0x18(sp)
8010d574:  lui s0,0x800a
8010d578:  lw v0,0x3294(s0)
8010d57c:  nop
8010d580:  beq v0,zero,0x8010d598
8010d584:  _lui a0,0x8011
8010d588:  jal 0x8004ba54
8010d58c:  _nop
8010d590:  sw zero,0x3294(s0)
8010d594:  lui a0,0x8011
8010d598:  addiu a0,a0,0x33fc
8010d59c:  addiu s0,a0,-0x3c
8010d5a0:  lw v0,0x24(s0)
8010d5a4:  nop
8010d5a8:  sll v0,v0,0x2
8010d5ac:  addu v0,v0,s0
8010d5b0:  lw a1,0x1c(v0)
8010d5b4:  jal 0x8004f82c
8010d5b8:  _nop
8010d5bc:  lw v1,0x24(s0)
8010d5c0:  lhu v0,0x3c(s0)
8010d5c4:  lhu a1,0x40(s0)
8010d5c8:  lw a0,0x38(s0)
8010d5cc:  li a3,0x1
8010d5d0:  subu a2,a3,v1
8010d5d4:  addu v0,v0,a1
8010d5d8:  sll a0,a0,0x3
8010d5dc:  addu a0,a0,s0
8010d5e0:  sw a2,0x24(s0)
8010d5e4:  sh v0,0x3c(s0)
8010d5e8:  lh v1,0x28(a0)
8010d5ec:  lh a0,0x2c(a0)
8010d5f0:  sll v0,v0,0x10
8010d5f4:  sra v0,v0,0x10
8010d5f8:  addu v1,v1,a0
8010d5fc:  slt v0,v0,v1
8010d600:  beq v0,zero,0x8010d640
8010d604:  _sll a0,a1,0x10
8010d608:  lh v0,0x42(s0)
8010d60c:  sra a0,a0,0x10
8010d610:  li v1,-0x10
8010d614:  addiu v0,v0,0xf
8010d618:  and v0,v0,v1
8010d61c:  mult a0,v0
8010d620:  sll v0,a2,0x2
8010d624:  addu v0,v0,s0
8010d628:  lw a0,0x1c(v0)
8010d62c:  mflo t0
8010d630:  sltiu a0,zero,0x38cc
8010d634:  srav a1,t0,a3
8010d638:  slti a0,zero,0x3591
8010d63c:  nop
8010d640:  sw a3,0x44(s0)
8010d644:  lw ra,0x1c(sp)
8010d648:  lw s0,0x18(sp)
8010d64c:  jr ra
8010d650:  _addiu sp,sp,0x20
