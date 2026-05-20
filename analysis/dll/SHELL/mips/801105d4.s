# addr: 0x801105d4  name: FUN_801105d4
801105d4:  addiu sp,sp,-0x58
801105d8:  lui v1,0x8011
801105dc:  addiu v1,v1,0x3438
801105e0:  sw ra,0x50(sp)
801105e4:  lw v0,0x0(v1)
801105e8:  nop
801105ec:  bne v0,zero,0x80110640
801105f0:  _li v0,0x1
801105f4:  lw a1,0xc(v1)
801105f8:  lui v1,0x8011
801105fc:  lw v1,0x3434(v1)
80110600:  sllv v0,v0,a1
80110604:  or v1,v1,v0
80110608:  addiu a1,sp,0x10
8011060c:  lui at,0x8011
80110610:  sltiu a0,zero,0x41fb
80110614:  sw v1,0x3434(at)
80110618:  sltiu a0,zero,0x4420
8011061c:  nop
80110620:  sltiu a0,zero,0x4244
80110624:  addiu a0,sp,0x10
80110628:  sltiu a0,zero,0x4498
8011062c:  nop
80110630:  sltiu a0,zero,0x41c5
80110634:  move a0,v0
80110638:  slti a0,zero,0x4194
8011063c:  nop
80110640:  lui a0,0x8010
80110644:  jal 0x80052604
80110648:  _addiu a0,a0,0x124c
8011064c:  li v0,-0x1
80110650:  lw ra,0x50(sp)
80110654:  addiu sp,sp,0x58
80110658:  jr ra
8011065c:  _nop
