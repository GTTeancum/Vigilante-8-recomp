# addr: 0x8010eefc  name: FUN_8010eefc
8010eefc:  addiu sp,sp,-0x18
8010ef00:  lui v1,0x8011
8010ef04:  addiu v1,v1,0x3438
8010ef08:  sw ra,0x10(sp)
8010ef0c:  lw v0,0x0(v1)
8010ef10:  nop
8010ef14:  bne v0,zero,0x8010ef0c
8010ef18:  _li a0,0x7
8010ef1c:  jal 0x80048118
8010ef20:  _clear a1
8010ef24:  sltiu a0,zero,0x43f3
8010ef28:  nop
8010ef2c:  lw ra,0x10(sp)
8010ef30:  addiu sp,sp,0x18
8010ef34:  jr ra
8010ef38:  _nop
