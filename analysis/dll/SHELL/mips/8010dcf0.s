# addr: 0x8010dcf0  name: FUN_8010dcf0
8010dcf0:  addiu sp,sp,-0x18
8010dcf4:  sw ra,0x10(sp)
8010dcf8:  jal 0x80053a24
8010dcfc:  _nop
8010dd00:  lui v1,0x8006
8010dd04:  lw v1,0xa0(v1)
8010dd08:  li v0,0x1
8010dd0c:  bne v1,v0,0x8010dd2c
8010dd10:  _nop
8010dd14:  sltiu a0,zero,0x37c7
8010dd18:  clear a0
8010dd1c:  sltiu a0,zero,0x37bd
8010dd20:  clear a0
8010dd24:  slti a0,zero,0x374f
8010dd28:  nop
8010dd2c:  jal 0x800493ec
8010dd30:  _clear a0
8010dd34:  jal 0x80048fbc
8010dd38:  _clear a0
8010dd3c:  lui v0,0x8011
8010dd40:  lw v0,0x2374(v0)
8010dd44:  nop
8010dd48:  sb zero,0x0(v0)
8010dd4c:  lui v0,0x8011
8010dd50:  lw v0,0x2380(v0)
8010dd54:  nop
8010dd58:  jal 0x80053a34
8010dd5c:  _sb zero,0x0(v0)
8010dd60:  lw ra,0x10(sp)
8010dd64:  addiu sp,sp,0x18
8010dd68:  jr ra
8010dd6c:  _nop
