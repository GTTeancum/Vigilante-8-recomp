# addr: 0x8004f5e8  name: SYS_OBJ_3E4
8004f5e8:  addiu sp,sp,-0x20
8004f5ec:  move t0,a0
8004f5f0:  sw s0,0x18(sp)
8004f5f4:  lui a0,0x8006
8004f5f8:  addiu a0,a0,0x5026
8004f5fc:  sw ra,0x1c(sp)
8004f600:  lbu v1,0x0(a0)
8004f604:  li v0,0x1
8004f608:  beq v1,v0,0x8004f624
8004f60c:  _move s0,a1
8004f610:  li v0,0x2
8004f614:  beq v1,v0,0x8004f6b0
8004f618:  _nop
8004f61c:  j 0x8004f6f4
8004f620:  _nop
8004f624:  lh a1,0x4(s0)
8004f628:  lh v1,0x2(a0)
8004f62c:  nop
8004f630:  slt v0,v1,a1
8004f634:  bne v0,zero,0x8004f6a4
8004f638:  _nop
8004f63c:  lh a3,0x0(s0)
8004f640:  nop
8004f644:  addu v0,a1,a3
8004f648:  slt v0,v1,v0
8004f64c:  bne v0,zero,0x8004f6a4
8004f650:  _nop
8004f654:  lh v1,0x2(s0)
8004f658:  lh a0,0x4(a0)
8004f65c:  nop
8004f660:  slt v0,a0,v1
8004f664:  bne v0,zero,0x8004f6a4
8004f668:  _nop
8004f66c:  lh a2,0x6(s0)
8004f670:  nop
8004f674:  addu v0,v1,a2
8004f678:  slt v0,a0,v0
8004f67c:  bne v0,zero,0x8004f6a4
8004f680:  _nop
8004f684:  blez a1,0x8004f6a4
8004f688:  _nop
8004f68c:  bltz a3,0x8004f6a4
8004f690:  _nop
8004f694:  bltz v1,0x8004f6a4
8004f698:  _nop
8004f69c:  bgtz a2,0x8004f6f4
8004f6a0:  _nop
8004f6a4:  lui a0,0x8001
8004f6a8:  j 0x8004f6b8
8004f6ac:  _addiu a0,a0,0x1250
8004f6b0:  lui a0,0x8001
8004f6b4:  addiu a0,a0,0x1270
