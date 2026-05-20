# addr: 0x8002be84  name: FUN_8002be84
8002be84:  addiu sp,sp,-0x60
8002be88:  sw s1,0x54(sp)
8002be8c:  move s1,a0
8002be90:  li a1,0x12c
8002be94:  sw ra,0x58(sp)
8002be98:  jal 0x80020890
8002be9c:  _sw s0,0x50(sp)
8002bea0:  lbu a0,0xd3(s1)
8002bea4:  lui v0,0x8003
8002bea8:  addiu v0,v0,-0x4230
8002beac:  sw v0,0x64(s1)
8002beb0:  sh zero,0xc(s1)
8002beb4:  sh zero,0xa6(s1)
8002beb8:  jal 0x800441c8
8002bebc:  _sb zero,0xb2(s1)
8002bec0:  lb a0,0x5(s1)
8002bec4:  nop
8002bec8:  bne a0,zero,0x8002bee4
8002becc:  _nop
8002bed0:  jal 0x8004410c
8002bed4:  _nop
8002bed8:  sb v0,0x5(s1)
8002bedc:  sll v0,v0,0x18
8002bee0:  sra a0,v0,0x18
8002bee4:  lw a1,0x5f8(gp)
8002bee8:  li a2,0x22
8002beec:  jal 0x8004483c
8002bef0:  _addiu a3,s1,0x24
8002bef4:  lh v0,0x6(s1)
8002bef8:  nop
8002befc:  bgtz v0,0x8002bf14
8002bf00:  _li v0,0x3
8002bf04:  lb v1,0x15(gp)
8002bf08:  nop
8002bf0c:  bne v1,v0,0x8002bf50
8002bf10:  _lui v0,0x8007
8002bf14:  jal 0x8002b940
8002bf18:  _move a0,s1
8002bf1c:  addiu a0,sp,0x10
8002bf20:  lui a1,0x8001
8002bf24:  addiu a1,a1,0x51c
8002bf28:  jal 0x80053004
8002bf2c:  _move a2,v0
8002bf30:  lh v0,0x6(s1)
8002bf34:  nop
8002bf38:  bgtz v0,0x8002bf44
8002bf3c:  _clear a0
8002bf40:  addiu a0,v0,0x3
8002bf44:  jal 0x800129e8
8002bf48:  _addiu a1,sp,0x10
8002bf4c:  lui v0,0x8007
8002bf50:  lw a0,0x37d8(v0)
8002bf54:  li a1,0x6
8002bf58:  lui a2,0x8006
8002bf5c:  jal 0x800407b4
8002bf60:  _addiu a2,a2,0x5710
8002bf64:  move s0,v0
8002bf68:  lw v1,0x0(s0)
8002bf6c:  move a0,s1
8002bf70:  ori a1,zero,0x8101
8002bf74:  lui v0,0x2
8002bf78:  or v1,v1,v0
8002bf7c:  jal 0x8001b038
8002bf80:  _sw v1,0x0(s0)
8002bf84:  move a0,s1
8002bf88:  move a1,v0
8002bf8c:  jal 0x8001b2fc
8002bf90:  _move a2,s0
8002bf94:  jal 0x80020744
8002bf98:  _move a0,s0
8002bf9c:  jal 0x800207c4
8002bfa0:  _move a0,s0
8002bfa4:  lw ra,0x58(sp)
8002bfa8:  lw s1,0x54(sp)
8002bfac:  lw s0,0x50(sp)
8002bfb0:  jr ra
8002bfb4:  _addiu sp,sp,0x60
