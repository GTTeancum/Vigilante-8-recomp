# addr: 0x8010eeac  name: FUN_8010eeac
8010eeac:  addiu sp,sp,-0x18
8010eeb0:  sw ra,0x10(sp)
8010eeb4:  sltiu a0,zero,0x42fc
8010eeb8:  nop
8010eebc:  lui v0,0x8011
8010eec0:  addiu v0,v0,0x3438
8010eec4:  li v1,-0x1
8010eec8:  sw zero,0x0(v0)
8010eecc:  sw zero,0x4(v0)
8010eed0:  sw zero,0x8(v0)
8010eed4:  sltiu a0,zero,0x4374
8010eed8:  sw v1,0x10(v0)
8010eedc:  lui a1,0x8011
8010eee0:  addiu a1,a1,0x768
8010eee4:  jal 0x80048118
8010eee8:  _li a0,0x7
8010eeec:  lw ra,0x10(sp)
8010eef0:  addiu sp,sp,0x18
8010eef4:  jr ra
8010eef8:  _nop
