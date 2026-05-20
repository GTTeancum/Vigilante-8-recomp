# addr: 0x80053c90  name: PadGetState
80053c90:  lui v0,0x8006
80053c94:  lw v0,0x5270(v0)
80053c98:  addiu sp,sp,-0x18
80053c9c:  sw ra,0x10(sp)
80053ca0:  jalr v0
80053ca4:  _nop
80053ca8:  move a0,v0
80053cac:  lw v0,0x34(a0)
80053cb0:  lui v1,0xffff
80053cb4:  and v0,v0,v1
80053cb8:  bne v0,zero,0x80053cf8
80053cbc:  _nop
80053cc0:  lw v0,0x10(a0)
80053cc4:  nop
80053cc8:  beq a0,v0,0x80053ce0
80053ccc:  _nop
80053cd0:  lbu v0,0x38(a0)
80053cd4:  nop
80053cd8:  bne v0,zero,0x80053cf8
80053cdc:  _nop
80053ce0:  lw v0,0x30(a0)
80053ce4:  nop
80053ce8:  lbu v0,0x0(v0)
80053cec:  nop
80053cf0:  beq v0,zero,0x80053d3c
80053cf4:  _nop
80053cf8:  lbu v1,0x49(a0)
80053cfc:  li v0,0x3
80053d00:  beq v1,v0,0x80053d34
80053d04:  _slti v0,v1,0x4
80053d08:  beq v0,zero,0x80053d20
80053d0c:  _li v0,0x2
80053d10:  beq v1,v0,0x80053d40
80053d14:  _li v0,0x1
80053d18:  j 0x80053d3c
80053d1c:  _nop
80053d20:  li v0,0x6
80053d24:  beq v1,v0,0x80053d40
80053d28:  _li v0,0x4
80053d2c:  j 0x80053d3c
80053d30:  _nop
80053d34:  j 0x80053d40
80053d38:  _li v0,0x1
