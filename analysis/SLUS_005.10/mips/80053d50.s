# addr: 0x80053d50  name: PadInfoMode
80053d50:  lui v0,0x8006
80053d54:  lw v0,0x5270(v0)
80053d58:  addiu sp,sp,-0x20
80053d5c:  sw s0,0x10(sp)
80053d60:  move s0,a1
80053d64:  sw s1,0x14(sp)
80053d68:  sw ra,0x18(sp)
80053d6c:  jalr v0
80053d70:  _move s1,a2
80053d74:  move v1,v0
80053d78:  li v0,0x3
80053d7c:  beq s0,v0,0x80053dd8
80053d80:  _slti v0,s0,0x4
80053d84:  beq v0,zero,0x80053da4
80053d88:  _li v0,0x1
80053d8c:  beq s0,v0,0x80053dc0
80053d90:  _li v0,0x2
80053d94:  beq s0,v0,0x80053dcc
80053d98:  _clear v0
80053d9c:  j 0x80053e34
80053da0:  _nop
80053da4:  li v0,0x4
80053da8:  beq s0,v0,0x80053de4
80053dac:  _li v0,0x64
80053db0:  beq s0,v0,0x80053e24
80053db4:  _clear v0
80053db8:  j 0x80053e34
80053dbc:  _nop
80053dc0:  lbu v0,0xe8(v1)
80053dc4:  j 0x80053e34
80053dc8:  _nop
80053dcc:  lhu v0,0xe6(v1)
80053dd0:  j 0x80053e34
80053dd4:  _nop
80053dd8:  lbu v0,0xe4(v1)
80053ddc:  j 0x80053e34
80053de0:  _nop
80053de4:  bgez s1,0x80053df8
80053de8:  _nop
80053dec:  lbu v0,0xe3(v1)
80053df0:  j 0x80053e34
80053df4:  _nop
80053df8:  lbu v0,0xe3(v1)
80053dfc:  nop
80053e00:  slt v0,s1,v0
80053e04:  beq v0,zero,0x80053e30
80053e08:  _sll v0,s1,0x1
80053e0c:  lw v1,0x0(v1)
80053e10:  nop
80053e14:  addu v0,v0,v1
80053e18:  lhu v0,0x0(v0)
80053e1c:  j 0x80053e34
80053e20:  _nop
80053e24:  lw v0,0x4c(v1)
80053e28:  j 0x80053e34
80053e2c:  _nop
80053e30:  clear v0
