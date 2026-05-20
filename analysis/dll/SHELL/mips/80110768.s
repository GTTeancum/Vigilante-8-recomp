# addr: 0x80110768  name: FUN_80110768
80110768:  addiu sp,sp,-0x18
8011076c:  sw ra,0x10(sp)
80110770:  sltiu a0,zero,0x433a
80110774:  nop
80110778:  bne v0,zero,0x801107dc
8011077c:  _nop
80110780:  sltiu a0,zero,0x431f
80110784:  nop
80110788:  sltiu a0,zero,0x433a
8011078c:  nop
80110790:  beq v0,zero,0x801107dc
80110794:  _li v0,0x1
80110798:  lui v1,0x8011
8011079c:  addiu v1,v1,0x3438
801107a0:  sw v0,0x8(v1)
801107a4:  lw v0,0x0(v1)
801107a8:  lui a1,0x8011
801107ac:  addiu a1,a1,0x3480
801107b0:  sw v0,0x0(a1)
801107b4:  lw v0,0x4(v1)
801107b8:  lw a2,0x40(v1)
801107bc:  sw v0,0x4(a1)
801107c0:  sw zero,0x0(v1)
801107c4:  beq a2,zero,0x801107dc
801107c8:  _sw zero,0x4(v1)
801107cc:  lw a0,0x0(a1)
801107d0:  lw a1,0x4(a1)
801107d4:  jalr a2
801107d8:  _nop
801107dc:  lw ra,0x10(sp)
801107e0:  addiu sp,sp,0x18
801107e4:  jr ra
801107e8:  _nop
