# addr: 0x8010f698  name: FUN_8010f698
8010f698:  addiu sp,sp,-0x20
8010f69c:  sw s1,0x14(sp)
8010f6a0:  move s1,a0
8010f6a4:  sw ra,0x1c(sp)
8010f6a8:  sw s2,0x18(sp)
8010f6ac:  sw s0,0x10(sp)
8010f6b0:  lw v1,0x0(s1)
8010f6b4:  li s2,0xa
8010f6b8:  beq v1,s2,0x8010f6f4
8010f6bc:  _slti v0,v1,0xb
8010f6c0:  beq v0,zero,0x8010f6d8
8010f6c4:  _li v0,0x1e
8010f6c8:  beq v1,zero,0x8010f6e8
8010f6cc:  _clear v0
8010f6d0:  slti a0,zero,0x3dff
8010f6d4:  nop
8010f6d8:  beq v1,v0,0x8010f74c
8010f6dc:  _clear v0
8010f6e0:  slti a0,zero,0x3dff
8010f6e4:  nop
8010f6e8:  lui at,0x8011
8010f6ec:  sw zero,0x333c(at)
8010f6f0:  sw s2,0x0(s1)
8010f6f4:  lui s0,0x8011
8010f6f8:  addiu s0,s0,0x3448
8010f6fc:  lw a0,0x0(s0)
8010f700:  lw a1,0x4(s0)
8010f704:  sltiu a0,zero,0x4238
8010f708:  clear a2
8010f70c:  lw v1,0x4(s0)
8010f710:  nop
8010f714:  bne v0,v1,0x8010f6fc
8010f718:  _nop
8010f71c:  sltiu a0,zero,0x4420
8010f720:  nop
8010f724:  lui s0,0x8011
8010f728:  addiu s0,s0,0x3448
8010f72c:  lw a0,0x0(s0)
8010f730:  lw a1,0xc(s0)
8010f734:  sltiu a0,zero,0x423c
8010f738:  li a2,0x80
8010f73c:  bne v0,zero,0x8010f72c
8010f740:  _li v0,0x1e
8010f744:  slti a0,zero,0x3dfe
8010f748:  sw v0,0x0(s1)
8010f74c:  sltiu a0,zero,0x44ce
8010f750:  nop
8010f754:  beq v0,zero,0x8010f7fc
8010f758:  _clear v0
8010f75c:  sltiu a0,zero,0x4462
8010f760:  nop
8010f764:  move a0,v0
8010f768:  beq a0,zero,0x8010f79c
8010f76c:  _nop
8010f770:  lui v0,0x8011
8010f774:  lw v0,0x333c(v0)
8010f778:  nop
8010f77c:  addiu v0,v0,0x1
8010f780:  lui at,0x8011
8010f784:  sw v0,0x333c(at)
8010f788:  slti v0,v0,0x4
8010f78c:  beq v0,zero,0x8010f7d8
8010f790:  _nop
8010f794:  slti a0,zero,0x3dfe
8010f798:  sw s2,0x0(s1)
8010f79c:  lui s0,0x8011
8010f7a0:  addiu s0,s0,0x3454
8010f7a4:  lw v0,0x0(s0)
8010f7a8:  lw v1,-0x4(s0)
8010f7ac:  lui at,0x8011
8010f7b0:  sw zero,0x333c(at)
8010f7b4:  addiu v0,v0,0x80
8010f7b8:  sw v0,0x0(s0)
8010f7bc:  lw v0,-0x8(s0)
8010f7c0:  addiu v1,v1,-0x80
8010f7c4:  sw v1,-0x4(s0)
8010f7c8:  addiu v0,v0,0x80
8010f7cc:  bgtz v1,0x8010f7f4
8010f7d0:  _sw v0,-0x8(s0)
8010f7d4:  clear a0
8010f7d8:  sltiu a0,zero,0x41c5
8010f7dc:  nop
8010f7e0:  lui v1,0x8011
8010f7e4:  addiu v1,v1,0x3438
8010f7e8:  sw v0,0x4(v1)
8010f7ec:  slti a0,zero,0x3dff
8010f7f0:  li v0,0x1
8010f7f4:  sw s2,0x0(s1)
8010f7f8:  clear v0
8010f7fc:  lw ra,0x1c(sp)
8010f800:  lw s2,0x18(sp)
8010f804:  lw s1,0x14(sp)
8010f808:  lw s0,0x10(sp)
8010f80c:  jr ra
8010f810:  _addiu sp,sp,0x20
