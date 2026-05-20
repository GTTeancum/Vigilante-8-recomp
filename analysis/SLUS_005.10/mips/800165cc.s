# addr: 0x800165cc  name: FUN_800165cc
800165cc:  lw v1,0x6bc(gp)
800165d0:  addiu sp,sp,-0x18
800165d4:  sw ra,0x14(sp)
800165d8:  beq v1,zero,0x80016668
800165dc:  _sw s0,0x10(sp)
800165e0:  bne a0,zero,0x80016610
800165e4:  _nop
800165e8:  lw v0,0x5dcc(v1)
800165ec:  nop
800165f0:  bne v0,zero,0x80016610
800165f4:  _nop
800165f8:  lw v0,0x5dcc(v1)
800165fc:  nop
80016600:  bne v0,zero,0x80016610
80016604:  _nop
80016608:  beq a0,zero,0x800165f8
8001660c:  _nop
80016610:  lw v0,0x6bc(gp)
80016614:  nop
80016618:  lw a0,0x5dd0(v0)
8001661c:  jal 0x800480e4
80016620:  _lui s0,0x8007
80016624:  lw v1,0x4(gp)
80016628:  li v0,0x1
8001662c:  addiu s0,s0,-0xdf8
80016630:  subu v0,v0,v1
80016634:  sll a0,v0,0x1
80016638:  addu a0,a0,v0
8001663c:  sll a0,a0,0x3
80016640:  subu a0,a0,v0
80016644:  sll a0,a0,0x2
80016648:  jal 0x8004fbe4
8001664c:  _addu a0,a0,s0
80016650:  lw a0,0x6bc(gp)
80016654:  jal 0x80045088
80016658:  _nop
8001665c:  sw zero,0x6bc(gp)
80016660:  sb zero,0x74(s0)
80016664:  sb zero,0x18(s0)
80016668:  lw ra,0x14(sp)
8001666c:  lw s0,0x10(sp)
80016670:  jr ra
80016674:  _addiu sp,sp,0x18
