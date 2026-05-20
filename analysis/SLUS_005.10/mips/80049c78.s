# addr: 0x80049c78  name: BIOS_OBJ_6C4
80049c78:  jal 0x800495b4
80049c7c:  _nop
80049c80:  move s0,v0
80049c84:  beq s0,zero,0x80049cf0
80049c88:  _andi v0,s0,0x4
80049c8c:  beq v0,zero,0x80049cbc
80049c90:  _andi v0,s0,0x2
80049c94:  lui v0,0x8006
80049c98:  lw v0,0x80(v0)
80049c9c:  nop
80049ca0:  beq v0,zero,0x80049cb8
80049ca4:  _nop
80049ca8:  lbu a0,0x0(s5)
80049cac:  lui a1,0x800a
80049cb0:  jalr v0
80049cb4:  _addiu a1,a1,0x3248
80049cb8:  andi v0,s0,0x2
80049cbc:  beq v0,zero,0x80049c78
80049cc0:  _nop
80049cc4:  lui v0,0x8006
80049cc8:  lw v0,0x7c(v0)
80049ccc:  nop
80049cd0:  beq v0,zero,0x80049c78
80049cd4:  _nop
80049cd8:  lbu a0,0x0(s2)
80049cdc:  lui a1,0x800a
80049ce0:  jalr v0
80049ce4:  _addiu a1,a1,0x3240
80049ce8:  j 0x80049c78
80049cec:  _nop
80049cf0:  lui v0,0x8006
80049cf4:  lw v0,0x344(v0)
80049cf8:  nop
80049cfc:  sb s1,0x0(v0)
80049d00:  lbu v0,0x0(s2)
80049d04:  nop
80049d08:  andi a2,v0,0xff
80049d0c:  beq a2,s3,0x80049d1c
80049d10:  _li v0,0x5
80049d14:  bne a2,v0,0x80049d58
80049d18:  _nop
80049d1c:  sb s3,0x0(s2)
80049d20:  move a1,s7
80049d24:  lui a0,0x800a
80049d28:  addiu a0,a0,0x3240
80049d2c:  beq a1,zero,0x80049d50
80049d30:  _li v1,0x7
80049d34:  li a3,-0x1
80049d38:  lbu v0,0x0(a0)
80049d3c:  addiu a0,a0,0x1
80049d40:  addiu v1,v1,-0x1
80049d44:  sb v0,0x0(a1)
80049d48:  bne v1,a3,0x80049d38
80049d4c:  _addiu a1,a1,0x1
80049d50:  j 0x80049d60
80049d54:  _move v0,a2
80049d58:  beq s6,zero,0x80049b90
80049d5c:  _clear v0
