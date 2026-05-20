# addr: 0x80100c1c  name: FUN_80100c1c
80100c1c:  addiu sp,sp,-0x28
80100c20:  sw s2,0x18(sp)
80100c24:  move s2,a0
80100c28:  addiu v1,a1,-0x1
80100c2c:  sltiu v0,v1,0x8
80100c30:  sw ra,0x20(sp)
80100c34:  sw s3,0x1c(sp)
80100c38:  sw s1,0x14(sp)
80100c3c:  beq v0,zero,0x80100e50
80100c40:  _sw s0,0x10(sp)
80100c44:  lui v0,0x8010
80100c48:  addiu v0,v0,0xb8
80100c4c:  sll v1,v1,0x2
80100c50:  addu v1,v1,v0
80100c54:  lw v0,0x0(v1)
80100c58:  nop
80100c5c:  jr v0
80100c60:  _nop
80100c64:  lw a0,0xc(a2)
80100c68:  lw v0,0x38(s2)
80100c6c:  nop
80100c70:  bne a0,v0,0x80100cb4
80100c74:  _li v0,0x7
80100c78:  jal 0x8002239c
80100c7c:  _move a1,a2
80100c80:  beq v0,zero,0x80100e50
80100c84:  _li v0,0x1e
80100c88:  sb v0,0x8(s2)
80100c8c:  lui v0,0x8006
80100c90:  lhu v0,0x59d0(v0)
80100c94:  move a0,s2
80100c98:  jal 0x800207c4
80100c9c:  _sh v0,0x46(s2)
80100ca0:  move a0,s2
80100ca4:  jal 0x80020890
80100ca8:  _li a1,0x78
80100cac:  slti a0,zero,0x395
80100cb0:  clear v0
80100cb4:  lw a1,0x0(a2)
80100cb8:  nop
80100cbc:  lbu v1,0x4(a1)
80100cc0:  nop
80100cc4:  bne v1,v0,0x80100e54
80100cc8:  _clear v0
80100ccc:  lhu a2,0xc(a1)
80100cd0:  lw v0,0x0(s2)
80100cd4:  lw s0,0x78(s2)
80100cd8:  andi v0,v0,0x4
80100cdc:  lhu s1,0x0(s0)
80100ce0:  beq v0,zero,0x80100cec
80100ce4:  _move a0,s2
80100ce8:  sh zero,0x0(s0)
80100cec:  jal 0x80022320
80100cf0:  _move a1,a2
80100cf4:  slti a0,zero,0x394
80100cf8:  sh s1,0x0(s0)
80100cfc:  lui v0,0x8006
80100d00:  lw a0,0x5a18(v0)
80100d04:  nop
80100d08:  lw s0,0x0(a0)
80100d0c:  nop
80100d10:  beq s0,zero,0x80100d78
80100d14:  _li s3,0x2
80100d18:  lui s1,0x4
80100d1c:  ori s1,s1,0xafff
80100d20:  lw a0,0x8(a0)
80100d24:  nop
80100d28:  lbu v0,0x4(a0)
80100d2c:  nop
80100d30:  bne v0,s3,0x80100d5c
80100d34:  _nop
80100d38:  lhu v0,0xc(a0)
80100d3c:  nop
80100d40:  beq v0,zero,0x80100d5c
80100d44:  _addiu a0,a0,0x24
80100d48:  jal 0x80016aac
80100d4c:  _addiu a1,s2,0x48
80100d50:  slt v0,s1,v0
80100d54:  beq v0,zero,0x80100d70
80100d58:  _nop
80100d5c:  move a0,s0
80100d60:  lw s0,0x0(s0)
80100d64:  nop
80100d68:  bne s0,zero,0x80100d20
80100d6c:  _nop
80100d70:  bne s0,zero,0x80100d8c
80100d74:  _li v0,0x1
80100d78:  move a0,s2
80100d7c:  jal 0x80020890
80100d80:  _li a1,0xf
80100d84:  slti a0,zero,0x395
80100d88:  clear v0
80100d8c:  sb v0,0x8(s2)
80100d90:  lbu v0,0x8(s2)
80100d94:  nop
80100d98:  addiu v0,v0,-0x1
80100d9c:  sb v0,0x8(s2)
80100da0:  sll v0,v0,0x18
80100da4:  bne v0,zero,0x80100dcc
80100da8:  _move a0,s2
80100dac:  lw s0,0x78(s2)
80100db0:  nop
80100db4:  lhu s1,0x0(s0)
80100db8:  jal 0x8003fc50
80100dbc:  _sh zero,0x0(s0)
80100dc0:  li v0,-0x1
80100dc4:  slti a0,zero,0x395
80100dc8:  sh s1,0x0(s0)
80100dcc:  jal 0x8004410c
80100dd0:  _nop
80100dd4:  lw v1,0x58(s2)
80100dd8:  nop
80100ddc:  lw a1,0x8(v1)
80100de0:  move a0,v0
80100de4:  li a2,0x2
80100de8:  jal 0x8004483c
80100dec:  _addiu a3,s2,0x24
80100df0:  slti a0,zero,0x395
80100df4:  clear v0
80100df8:  jal 0x800207f8
80100dfc:  _move a0,s2
80100e00:  lw a0,0x38(s2)
80100e04:  jal 0x800207c4
80100e08:  _nop
80100e0c:  lw v1,0x38(s2)
80100e10:  move a0,s2
80100e14:  li v0,0xe
80100e18:  jal 0x8003fbc8
80100e1c:  _sh v0,0xc(v1)
80100e20:  lw v1,0x58(s2)
80100e24:  lw a0,0x48(s2)
80100e28:  lw a2,0x0(v1)
80100e2c:  lw a1,0x50(s2)
80100e30:  sll v1,v0,0x3
80100e34:  subu v1,v1,v0
80100e38:  sll v1,v1,0x2
80100e3c:  addiu v1,v1,0x1c
80100e40:  addu a2,a2,v1
80100e44:  jal 0x80025400
80100e48:  _sw a2,0x78(s2)
80100e4c:  sw v0,0x4c(s2)
80100e50:  clear v0
80100e54:  lw ra,0x20(sp)
80100e58:  lw s3,0x1c(sp)
80100e5c:  lw s2,0x18(sp)
80100e60:  lw s1,0x14(sp)
80100e64:  lw s0,0x10(sp)
80100e68:  jr ra
80100e6c:  _addiu sp,sp,0x28
