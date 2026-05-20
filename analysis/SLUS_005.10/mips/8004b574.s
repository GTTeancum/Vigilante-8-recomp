# addr: 0x8004b574  name: CdDiskReady
8004b574:  addiu sp,sp,-0x28
8004b578:  sw s0,0x18(sp)
8004b57c:  move s0,a0
8004b580:  li a0,0x1
8004b584:  clear a1
8004b588:  addiu a2,sp,0x10
8004b58c:  sw ra,0x20(sp)
8004b590:  jal 0x80049240
8004b594:  _sw s1,0x1c(sp)
8004b598:  lbu v0,0x10(sp)
8004b59c:  nop
8004b5a0:  andi v0,v0,0x10
8004b5a4:  bne v0,zero,0x8004b67c
8004b5a8:  _li v0,0x10
8004b5ac:  li a0,0x13
8004b5b0:  clear a1
8004b5b4:  jal 0x80049240
8004b5b8:  _addiu a2,sp,0x10
8004b5bc:  move a0,v0
8004b5c0:  li v0,0x1
8004b5c4:  bne s0,v0,0x8004b5ec
8004b5c8:  _li s1,0x2
8004b5cc:  lbu v1,0x10(sp)
8004b5d0:  li v0,0x2
8004b5d4:  bne v1,v0,0x8004b678
8004b5d8:  _nop
8004b5dc:  bne a0,zero,0x8004b67c
8004b5e0:  _nop
8004b5e4:  j 0x8004b67c
8004b5e8:  _li v0,0x5
8004b5ec:  clear s0
8004b5f0:  lbu v1,0x10(sp)
8004b5f4:  nop
8004b5f8:  andi v0,v1,0x2
8004b5fc:  beq v0,zero,0x8004b64c
8004b600:  _nop
8004b604:  bne v1,s1,0x8004b614
8004b608:  _nop
8004b60c:  bne a0,zero,0x8004b67c
8004b610:  _li v0,0x2
8004b614:  jal 0x80047e44
8004b618:  _li a0,0x1e
8004b61c:  li a0,0x13
8004b620:  clear a1
8004b624:  jal 0x80049240
8004b628:  _addiu a2,sp,0x10
8004b62c:  lbu v1,0x10(sp)
8004b630:  nop
8004b634:  bne v1,s1,0x8004b614
8004b638:  _move a0,v0
8004b63c:  beq a0,zero,0x8004b614
8004b640:  _li v0,0x2
8004b644:  j 0x8004b67c
8004b648:  _nop
8004b64c:  jal 0x80047e44
8004b650:  _li a0,0x1e
8004b654:  li a0,0x13
8004b658:  clear a1
8004b65c:  jal 0x80049240
8004b660:  _addiu a2,sp,0x10
8004b664:  move a0,v0
8004b668:  addiu s0,s0,0x1
8004b66c:  slti v0,s0,0xa
8004b670:  bne v0,zero,0x8004b5f0
8004b674:  _nop
8004b678:  li v0,0x5
