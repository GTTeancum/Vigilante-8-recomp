# addr: 0x80110c7c  name: FUN_80110c7c
80110c7c:  lui v1,0x8011
80110c80:  lw v1,0x3354(v1)
80110c84:  addiu sp,sp,-0x18
80110c88:  bltz v1,0x80110cd8
80110c8c:  _sw ra,0x10(sp)
80110c90:  sll v0,v1,0x2
80110c94:  sll v1,v1,0x4
80110c98:  lui a0,0x8011
80110c9c:  addiu a0,a0,0x34c0
80110ca0:  lui at,0x8011
80110ca4:  addu at,at,v0
80110ca8:  lw v0,0x3500(at)
80110cac:  nop
80110cb0:  jalr v0
80110cb4:  _addu a0,v1,a0
80110cb8:  beq v0,zero,0x80110cd8
80110cbc:  _nop
80110cc0:  lui v0,0x8011
80110cc4:  lw v0,0x3354(v0)
80110cc8:  nop
80110ccc:  addiu v0,v0,-0x1
80110cd0:  lui at,0x8011
80110cd4:  sw v0,0x3354(at)
80110cd8:  lw ra,0x10(sp)
80110cdc:  addiu sp,sp,0x18
80110ce0:  jr ra
80110ce4:  _nop
