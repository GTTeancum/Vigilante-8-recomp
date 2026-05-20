# addr: 0x8010f5e0  name: FUN_8010f5e0
8010f5e0:  addiu sp,sp,-0x18
8010f5e4:  lui a3,0x8011
8010f5e8:  addiu a3,a3,0x3448
8010f5ec:  sw ra,0x10(sp)
8010f5f0:  lw v0,0x0(a3)
8010f5f4:  nop
8010f5f8:  bgez v0,0x8010f60c
8010f5fc:  _move t0,a0
8010f600:  lui a0,0x8010
8010f604:  slti a0,zero,0x3d9f
8010f608:  addiu a0,a0,0x11cc
8010f60c:  lw v0,-0x10(a3)
8010f610:  nop
8010f614:  beq v0,zero,0x8010f628
8010f618:  _addiu v1,a3,-0x10
8010f61c:  lui a0,0x8010
8010f620:  slti a0,zero,0x3d9f
8010f624:  addiu a0,a0,0x10e4
8010f628:  andi v0,a2,0x7f
8010f62c:  beq v0,zero,0x8010f640
8010f630:  _andi v0,a1,0x7f
8010f634:  lui a0,0x8010
8010f638:  slti a0,zero,0x3d9f
8010f63c:  addiu a0,a0,0x11f0
8010f640:  bne v0,zero,0x8010f674
8010f644:  _li v0,0x5
8010f648:  lui a0,0x8011
8010f64c:  addiu a0,a0,-0x968
8010f650:  sw v0,-0x10(a3)
8010f654:  sw zero,0x4(v1)
8010f658:  sw zero,0x8(v1)
8010f65c:  sw a1,0x14(v1)
8010f660:  sw t0,0x1c(v1)
8010f664:  sltiu a0,zero,0x4300
8010f668:  sw a2,0x18(v1)
8010f66c:  slti a0,zero,0x3da2
8010f670:  li v0,0x1
8010f674:  lui a0,0x8010
8010f678:  addiu a0,a0,0x121c
8010f67c:  jal 0x80052604
8010f680:  _nop
8010f684:  clear v0
8010f688:  lw ra,0x10(sp)
8010f68c:  addiu sp,sp,0x18
8010f690:  jr ra
8010f694:  _nop
