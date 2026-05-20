# addr: 0x80101574  name: FUN_80101574
80101574:  addiu sp,sp,-0x40
80101578:  sw s0,0x30(sp)
8010157c:  move s0,a0
80101580:  sw s2,0x38(sp)
80101584:  move s2,a1
80101588:  sw s1,0x34(sp)
8010158c:  lui s1,0x8010
80101590:  sw ra,0x3c(sp)
80101594:  move a0,s2
80101598:  addiu a1,s0,0x10
8010159c:  jal 0x8004cf04
801015a0:  _addiu a2,sp,0x10
801015a4:  jal 0x80043974
801015a8:  _addiu a0,sp,0x10
801015ac:  lw a0,0x30(s0)
801015b0:  nop
801015b4:  beq a0,zero,0x801015f4
801015b8:  _nop
801015bc:  lhu v0,0x0(a0)
801015c0:  nop
801015c4:  andi v0,v0,0x1
801015c8:  beq v0,zero,0x801015f4
801015cc:  _nop
801015d0:  sltiu a0,zero,0x4c7
801015d4:  addiu a1,s1,0x11a0
801015d8:  lw v1,0x30(s0)
801015dc:  nop
801015e0:  lhu v0,0x0(v1)
801015e4:  nop
801015e8:  andi v0,v0,0xfffe
801015ec:  ori v0,v0,0x4
801015f0:  sh v0,0x0(v1)
801015f4:  lw a0,0x68(s0)
801015f8:  nop
801015fc:  beq a0,zero,0x8010163c
80101600:  _nop
80101604:  lhu v0,0x0(a0)
80101608:  nop
8010160c:  andi v0,v0,0x1
80101610:  beq v0,zero,0x8010163c
80101614:  _nop
80101618:  sltiu a0,zero,0x4c7
8010161c:  addiu a1,s1,0x11a0
80101620:  lw v1,0x68(s0)
80101624:  nop
80101628:  lhu v0,0x0(v1)
8010162c:  nop
80101630:  andi v0,v0,0xfffe
80101634:  ori v0,v0,0x4
80101638:  sh v0,0x0(v1)
8010163c:  lw a0,0x38(s0)
80101640:  nop
80101644:  beq a0,zero,0x80101654
80101648:  _nop
8010164c:  sltiu a0,zero,0x55d
80101650:  addiu a1,sp,0x10
80101654:  lw s0,0x34(s0)
80101658:  nop
8010165c:  bne s0,zero,0x80101598
80101660:  _move a0,s2
80101664:  lw ra,0x3c(sp)
80101668:  lw s2,0x38(sp)
8010166c:  lw s1,0x34(sp)
80101670:  lw s0,0x30(sp)
80101674:  jr ra
80101678:  _addiu sp,sp,0x40
