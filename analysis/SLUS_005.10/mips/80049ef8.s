# addr: 0x80049ef8  name: BIOS_OBJ_944
80049ef8:  jal 0x800495b4
80049efc:  _nop
80049f00:  move s0,v0
80049f04:  beq s0,zero,0x80049f70
80049f08:  _andi v0,s0,0x4
80049f0c:  beq v0,zero,0x80049f3c
80049f10:  _andi v0,s0,0x2
80049f14:  lui v0,0x8006
80049f18:  lw v0,0x80(v0)
80049f1c:  nop
80049f20:  beq v0,zero,0x80049f38
80049f24:  _nop
80049f28:  lbu a0,0x0(s6)
80049f2c:  lui a1,0x800a
80049f30:  jalr v0
80049f34:  _addiu a1,a1,0x3248
80049f38:  andi v0,s0,0x2
80049f3c:  beq v0,zero,0x80049ef8
80049f40:  _nop
80049f44:  lui v0,0x8006
80049f48:  lw v0,0x7c(v0)
80049f4c:  nop
80049f50:  beq v0,zero,0x80049ef8
80049f54:  _nop
80049f58:  lbu a0,0x0(s2)
80049f5c:  lui a1,0x800a
80049f60:  jalr v0
80049f64:  _addiu a1,a1,0x3240
80049f68:  j 0x80049ef8
80049f6c:  _nop
80049f70:  lui v0,0x8006
80049f74:  lw v0,0x344(v0)
80049f78:  nop
80049f7c:  sb s1,0x0(v0)
80049f80:  lbu v0,0x0(s4)
80049f84:  nop
80049f88:  andi a2,v0,0xff
80049f8c:  beq a2,zero,0x80049fd0
80049f90:  _nop
80049f94:  sb zero,0x2(s2)
80049f98:  lui a0,0x800a
80049f9c:  addiu a0,a0,0x3250
80049fa0:  beq s3,zero,0x8004a018
80049fa4:  _move a1,s3
80049fa8:  li v1,0x7
80049fac:  li a3,-0x1
80049fb0:  lbu v0,0x0(a0)
80049fb4:  addiu a0,a0,0x1
80049fb8:  addiu v1,v1,-0x1
80049fbc:  sb v0,0x0(a1)
80049fc0:  bne v1,a3,0x80049fb0
80049fc4:  _addiu a1,a1,0x1
80049fc8:  j 0x8004a028
80049fcc:  _move v0,a2
80049fd0:  lbu v0,-0x1(s4)
80049fd4:  nop
80049fd8:  andi a2,v0,0xff
80049fdc:  beq a2,zero,0x8004a020
80049fe0:  _nop
80049fe4:  sb zero,0x1(s2)
80049fe8:  move a1,s3
80049fec:  lui a0,0x800a
80049ff0:  addiu a0,a0,0x3248
80049ff4:  beq a1,zero,0x8004a018
80049ff8:  _li v1,0x7
80049ffc:  li a3,-0x1
8004a000:  lbu v0,0x0(a0)
8004a004:  addiu a0,a0,0x1
8004a008:  addiu v1,v1,-0x1
8004a00c:  sb v0,0x0(a1)
8004a010:  bne v1,a3,0x8004a000
8004a014:  _addiu a1,a1,0x1
8004a018:  j 0x8004a028
8004a01c:  _move v0,a2
8004a020:  beq s7,zero,0x80049e10
8004a024:  _clear v0
