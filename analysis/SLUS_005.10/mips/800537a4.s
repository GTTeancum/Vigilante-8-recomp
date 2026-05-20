# addr: 0x800537a4  name: SPRINTF_OBJ_7A0
800537a4:  lw v0,0x214(sp)
800537a8:  nop
800537ac:  slt v0,s0,v0
800537b0:  beq v0,zero,0x800537f4
800537b4:  _addu a0,s3,s2
800537b8:  lw v0,0x210(sp)
800537bc:  nop
800537c0:  andi v0,v0,0x1
800537c4:  bne v0,zero,0x800537f8
800537c8:  _move a1,s1
800537cc:  addu v0,s3,s2
800537d0:  sb s4,0x0(v0)
800537d4:  lw v0,0x214(sp)
800537d8:  nop
800537dc:  addiu v0,v0,-0x1
800537e0:  sw v0,0x214(sp)
800537e4:  slt v0,s0,v0
800537e8:  bne v0,zero,0x800537cc
800537ec:  _addiu s2,s2,0x1
800537f0:  addu a0,s3,s2
800537f4:  move a1,s1
800537f8:  jal 0x80044d9c
800537fc:  _move a2,s0
80053800:  lw v0,0x214(sp)
80053804:  nop
80053808:  slt v0,s0,v0
8005380c:  beq v0,zero,0x80053830
80053810:  _addu s2,s2,s0
80053814:  addu v0,s3,s2
80053818:  sb s4,0x0(v0)
8005381c:  lw v0,0x214(sp)
80053820:  addiu s0,s0,0x1
80053824:  slt v0,s0,v0
80053828:  bne v0,zero,0x80053814
8005382c:  _addiu s2,s2,0x1
