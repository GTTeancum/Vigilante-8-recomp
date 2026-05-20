# addr: 0x80054cb0  name: _padSendAtLoadInfo
80054cb0:  addiu sp,sp,-0x18
80054cb4:  sw ra,0x10(sp)
80054cb8:  lbu v1,0x46(a0)
80054cbc:  li v0,0x3
80054cc0:  beq v1,v0,0x80054d04
80054cc4:  _slti v0,v1,0x4
80054cc8:  beq v0,zero,0x80054ce0
80054ccc:  _li v0,0x2
80054cd0:  beq v1,v0,0x80054cf4
80054cd4:  _nop
80054cd8:  j 0x80054d24
80054cdc:  _nop
80054ce0:  li v0,0x4
80054ce4:  beq v1,v0,0x80054d18
80054ce8:  _nop
80054cec:  j 0x80054d24
80054cf0:  _nop
80054cf4:  jal 0x80055560
80054cf8:  _nop
80054cfc:  j 0x80054d24
80054d00:  _nop
80054d04:  lbu a1,0xe4(a0)
80054d08:  jal 0x80055574
80054d0c:  _nop
80054d10:  j 0x80054d24
80054d14:  _nop
80054d18:  lbu a1,0x47(a0)
80054d1c:  jal 0x800555b4
80054d20:  _nop
