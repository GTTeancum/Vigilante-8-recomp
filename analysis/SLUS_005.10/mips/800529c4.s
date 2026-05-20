# addr: 0x800529c4  name: PRNT_OBJ_380
800529c4:  andi v0,s3,0x1
800529c8:  bne v0,zero,0x800529e8
800529cc:  _andi v0,s3,0x4
800529d0:  beq v0,zero,0x800529e8
800529d4:  _li a1,0x10
800529d8:  addiu s1,s1,0x4
800529dc:  lh v1,-0x4(s1)
800529e0:  j 0x800529f8
800529e4:  _andi v0,s3,0x8
800529e8:  addiu s1,s1,0x4
800529ec:  lw v1,-0x4(s1)
800529f0:  li a1,0x10
800529f4:  andi v0,s3,0x8
