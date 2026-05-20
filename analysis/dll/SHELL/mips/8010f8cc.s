# addr: 0x8010f8cc  name: FUN_8010f8cc
8010f8cc:  addiu sp,sp,-0x20
8010f8d0:  sw s1,0x14(sp)
8010f8d4:  move s1,a0
8010f8d8:  sw ra,0x1c(sp)
8010f8dc:  sw s2,0x18(sp)
8010f8e0:  sw s0,0x10(sp)
8010f8e4:  lw v1,0x0(s1)
8010f8e8:  li s2,0xa
8010f8ec:  beq v1,s2,0x8010f928
8010f8f0:  _slti v0,v1,0xb
8010f8f4:  beq v0,zero,0x8010f90c
8010f8f8:  _li v0,0x1e
8010f8fc:  beq v1,zero,0x8010f91c
8010f900:  _clear v0
8010f904:  slti a0,zero,0x3e8c
8010f908:  nop
8010f90c:  beq v1,v0,0x8010f980
8010f910:  _clear v0
8010f914:  slti a0,zero,0x3e8c
8010f918:  nop
8010f91c:  lui at,0x8011
8010f920:  sw zero,0x3340(at)
8010f924:  sw s2,0x0(s1)
8010f928:  lui s0,0x8011
8010f92c:  addiu s0,s0,0x3448
8010f930:  lw a0,0x0(s0)
8010f934:  lw a1,0x4(s0)
8010f938:  sltiu a0,zero,0x4238
8010f93c:  clear a2
8010f940:  lw v1,0x4(s0)
8010f944:  nop
8010f948:  bne v0,v1,0x8010f930
8010f94c:  _nop
8010f950:  sltiu a0,zero,0x4420
8010f954:  nop
8010f958:  lui s0,0x8011
8010f95c:  addiu s0,s0,0x3448
8010f960:  lw a0,0x0(s0)
8010f964:  lw a1,0xc(s0)
8010f968:  jal 0x80053a44
8010f96c:  _li a2,0x80
8010f970:  bne v0,zero,0x8010f960
8010f974:  _li v0,0x1e
8010f978:  slti a0,zero,0x3e8b
8010f97c:  sw v0,0x0(s1)
8010f980:  sltiu a0,zero,0x44ce
8010f984:  nop
8010f988:  beq v0,zero,0x8010fa30
8010f98c:  _clear v0
8010f990:  sltiu a0,zero,0x4462
8010f994:  nop
8010f998:  move a0,v0
8010f99c:  beq a0,zero,0x8010f9d0
8010f9a0:  _nop
8010f9a4:  lui v0,0x8011
8010f9a8:  lw v0,0x3340(v0)
8010f9ac:  nop
8010f9b0:  addiu v0,v0,0x1
8010f9b4:  lui at,0x8011
8010f9b8:  sw v0,0x3340(at)
8010f9bc:  slti v0,v0,0x4
8010f9c0:  beq v0,zero,0x8010fa0c
8010f9c4:  _nop
8010f9c8:  slti a0,zero,0x3e8b
8010f9cc:  sw s2,0x0(s1)
8010f9d0:  lui s0,0x8011
8010f9d4:  addiu s0,s0,0x3454
8010f9d8:  lw v0,0x0(s0)
8010f9dc:  lw v1,-0x4(s0)
8010f9e0:  lui at,0x8011
8010f9e4:  sw zero,0x3340(at)
8010f9e8:  addiu v0,v0,0x80
8010f9ec:  sw v0,0x0(s0)
8010f9f0:  lw v0,-0x8(s0)
8010f9f4:  addiu v1,v1,-0x80
8010f9f8:  sw v1,-0x4(s0)
8010f9fc:  addiu v0,v0,0x80
8010fa00:  bgtz v1,0x8010fa28
8010fa04:  _sw v0,-0x8(s0)
8010fa08:  clear a0
8010fa0c:  sltiu a0,zero,0x41c5
8010fa10:  nop
8010fa14:  lui v1,0x8011
8010fa18:  addiu v1,v1,0x3438
8010fa1c:  sw v0,0x4(v1)
8010fa20:  slti a0,zero,0x3e8c
8010fa24:  li v0,0x1
8010fa28:  sw s2,0x0(s1)
8010fa2c:  clear v0
8010fa30:  lw ra,0x1c(sp)
8010fa34:  lw s2,0x18(sp)
8010fa38:  lw s1,0x14(sp)
8010fa3c:  lw s0,0x10(sp)
8010fa40:  jr ra
8010fa44:  _addiu sp,sp,0x20
