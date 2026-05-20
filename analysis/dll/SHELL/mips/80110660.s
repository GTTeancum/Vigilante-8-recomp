# addr: 0x80110660  name: FUN_80110660
80110660:  addiu sp,sp,-0xa0
80110664:  lui v0,0x8011
80110668:  addiu v0,v0,0x3438
8011066c:  sw ra,0x98(sp)
80110670:  sw s1,0x94(sp)
80110674:  sw s0,0x90(sp)
80110678:  lw v0,0x0(v0)
8011067c:  nop
80110680:  beq v0,zero,0x801106a4
80110684:  _move s1,a0
80110688:  lui a0,0x8010
8011068c:  jal 0x80052604
80110690:  _addiu a0,a0,0x124c
80110694:  slti a0,zero,0x41c0
80110698:  li v0,-0x1
8011069c:  slti a0,zero,0x41c0
801106a0:  clear v0
801106a4:  li v1,-0x1
801106a8:  li s0,0x7f
801106ac:  addiu v0,sp,0x8f
801106b0:  sb v1,0x0(v0)
801106b4:  addiu s0,s0,-0x1
801106b8:  bgez s0,0x801106b0
801106bc:  _addiu v0,v0,-0x1
801106c0:  clear s0
801106c4:  sltiu a0,zero,0x4420
801106c8:  nop
801106cc:  sltiu a0,zero,0x4220
801106d0:  nop
801106d4:  move a0,s1
801106d8:  move a1,s0
801106dc:  sltiu a0,zero,0x421c
801106e0:  addiu a2,sp,0x10
801106e4:  sltiu a0,zero,0x4498
801106e8:  nop
801106ec:  bne v0,zero,0x8011069c
801106f0:  _addiu s0,s0,0x1
801106f4:  slti v0,s0,0xf
801106f8:  bne v0,zero,0x801106c4
801106fc:  _li v0,0x1
80110700:  lw ra,0x98(sp)
80110704:  lw s1,0x94(sp)
80110708:  lw s0,0x90(sp)
8011070c:  jr ra
80110710:  _addiu sp,sp,0xa0
