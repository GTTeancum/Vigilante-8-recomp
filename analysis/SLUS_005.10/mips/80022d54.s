# addr: 0x80022d54  name: FUN_80022d54
80022d54:  addiu sp,sp,-0x28
80022d58:  sw s1,0x1c(sp)
80022d5c:  move s1,a0
80022d60:  sw ra,0x20(sp)
80022d64:  sw s0,0x18(sp)
80022d68:  lw a0,0x24(s1)
80022d6c:  lw a1,0x2c(s1)
80022d70:  jal 0x800244c4
80022d74:  _nop
80022d78:  andi v0,v0,0xffff
80022d7c:  beq v0,zero,0x80022db8
80022d80:  _addiu a0,s1,0xc0
80022d84:  lw a2,0xe4(s1)
80022d88:  addiu a1,s1,0x24
80022d8c:  lui a3,0x2
80022d90:  ori a3,a3,0x2740
80022d94:  sw zero,0x10(sp)
80022d98:  jal 0x80042ef0
80022d9c:  _addiu a2,a2,0x24
80022da0:  lw v1,0x0(s1)
80022da4:  li v0,0x3
80022da8:  sb v0,0x8(s1)
80022dac:  li v0,-0x21
80022db0:  and v1,v1,v0
80022db4:  sw v1,0x0(s1)
80022db8:  lw s0,0x10c(s1)
80022dbc:  li v0,0x3c
80022dc0:  sh zero,0xa4(s1)
80022dc4:  sh v0,0xa6(s1)
80022dc8:  lb v0,0x8(s0)
80022dcc:  nop
80022dd0:  bne v0,zero,0x80022e0c
80022dd4:  _li a1,0xb
80022dd8:  lw v0,0x64(s0)
80022ddc:  nop
80022de0:  beq v0,zero,0x80022dfc
80022de4:  _move a0,s0
80022de8:  li a1,0xc
80022dec:  jalr v0
80022df0:  _move a2,s1
80022df4:  j 0x80022e00
80022df8:  _nop
80022dfc:  clear v0
80022e00:  bne v0,zero,0x80022e0c
80022e04:  _li a1,0xb
80022e08:  li a1,0x4
80022e0c:  lw v0,0x64(s0)
80022e10:  nop
80022e14:  beq v0,zero,0x80022e24
80022e18:  _move a0,s0
80022e1c:  jalr v0
80022e20:  _move a2,s1
80022e24:  lw ra,0x20(sp)
80022e28:  lw s1,0x1c(sp)
80022e2c:  lw s0,0x18(sp)
80022e30:  jr ra
80022e34:  _addiu sp,sp,0x28
