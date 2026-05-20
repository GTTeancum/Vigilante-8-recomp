# addr: 0x80110c00  name: FUN_80110c00
80110c00:  lui v0,0x8011
80110c04:  lw v0,0x3354(v0)
80110c08:  addiu sp,sp,-0x18
80110c0c:  addiu a2,v0,0x1
80110c10:  slti v0,a2,0x4
80110c14:  bne v0,zero,0x80110c30
80110c18:  _sw ra,0x10(sp)
80110c1c:  lui a0,0x8010
80110c20:  jal 0x80052604
80110c24:  _addiu a0,a0,0x1274
80110c28:  slti a0,zero,0x431b
80110c2c:  nop
80110c30:  li a1,0x3
80110c34:  sll v0,a2,0x4
80110c38:  lui v1,0x8011
80110c3c:  addiu v1,v1,0x34cc
80110c40:  addu v1,v0,v1
80110c44:  sll v0,a2,0x2
80110c48:  lui at,0x8011
80110c4c:  sw a2,0x3354(at)
80110c50:  lui at,0x8011
80110c54:  addu at,at,v0
80110c58:  sw a0,0x3500(at)
80110c5c:  sw zero,0x0(v1)
80110c60:  addiu a1,a1,-0x1
80110c64:  bgez a1,0x80110c5c
80110c68:  _addiu v1,v1,-0x4
80110c6c:  lw ra,0x10(sp)
80110c70:  addiu sp,sp,0x18
80110c74:  jr ra
80110c78:  _nop
