# addr: 0x80100be8  name: FUN_80100be8
80100be8:  addiu sp,sp,-0x18
80100bec:  beq a1,zero,0x80100c08
80100bf0:  _sw ra,0x10(sp)
80100bf4:  li v0,0x1
80100bf8:  beq a1,v0,0x80100c34
80100bfc:  _li v0,0x3
80100c00:  slti a0,zero,0x310
80100c04:  nop
80100c08:  lhu v0,0x40(a0)
80100c0c:  lhu v1,0x42(a0)
80100c10:  addiu v0,v0,0x5b
80100c14:  addiu v1,v1,0x44
80100c18:  sh v0,0x40(a0)
80100c1c:  beq a2,zero,0x80100c40
80100c20:  _sh v1,0x42(a0)
80100c24:  jal 0x8001d708
80100c28:  _nop
80100c2c:  slti a0,zero,0x310
80100c30:  nop
80100c34:  sb v0,0x4(a0)
80100c38:  li v0,0x84
80100c3c:  sw v0,0x0(a0)
80100c40:  lw ra,0x10(sp)
80100c44:  clear v0
80100c48:  jr ra
80100c4c:  _addiu sp,sp,0x18
