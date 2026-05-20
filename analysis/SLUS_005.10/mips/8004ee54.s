# addr: 0x8004ee54  name: _patch_gte
8004ee54:  lui at,0x800a
8004ee58:  sw ra,0x32e8(at)
8004ee5c:  jal 0x80053a24
8004ee60:  _nop
8004ee64:  li t1,0x56
8004ee68:  li t2,0xb0
8004ee6c:  jalr t2
8004ee70:  _nop
8004ee74:  lw v0,0x18(v0)
8004ee78:  nop
8004ee7c:  addiu v0,v0,0x28
8004ee80:  move t7,v0
8004ee84:  lui t2,0x8005
8004ee88:  addiu t2,t2,-0x1100
8004ee8c:  lui t1,0x8005
8004ee90:  addiu t1,t1,-0x10e8
8004ee94:  lw v1,0x0(t2)
8004ee98:  lw t3,0x0(v0)
8004ee9c:  addiu t2,t2,0x4
8004eea0:  bne v1,t3,0x8004eedc
8004eea4:  _addiu v0,v0,0x4
8004eea8:  bne t2,t1,0x8004ee94
8004eeac:  _nop
8004eeb0:  move v0,t7
8004eeb4:  lui t2,0x8005
8004eeb8:  addiu t2,t2,-0x10e8
8004eebc:  lui t1,0x8005
8004eec0:  addiu t1,t1,-0x10d0
8004eec4:  lw v1,0x0(t2)
8004eec8:  nop
8004eecc:  sw v1,0x0(v0)
8004eed0:  addiu t2,t2,0x4
8004eed4:  bne t2,t1,0x8004eec4
8004eed8:  _addiu v0,v0,0x4
8004eedc:  jal 0x80053964
8004eee0:  _nop
8004eee4:  jal 0x80053a34
8004eee8:  _nop
8004eeec:  lui ra,0x800a
8004eef0:  lw ra,0x32e8(ra)
8004eef4:  nop
8004eef8:  jr ra
8004eefc:  _nop
