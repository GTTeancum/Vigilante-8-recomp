# addr: 0x80101d00  name: FUN_80101d00
80101d00:  addiu sp,sp,-0x30
80101d04:  sw s2,0x20(sp)
80101d08:  move s2,a0
80101d0c:  li a3,0x2
80101d10:  sw ra,0x28(sp)
80101d14:  sw s3,0x24(sp)
80101d18:  sw s1,0x1c(sp)
80101d1c:  beq a1,a3,0x80101e3c
80101d20:  _sw s0,0x18(sp)
80101d24:  sltiu v0,a1,0x3
80101d28:  beq v0,zero,0x80101d40
80101d2c:  _li v0,0x1
80101d30:  beq a1,v0,0x80101fa4
80101d34:  _clear v0
80101d38:  slti a0,zero,0x7f1
80101d3c:  nop
80101d40:  li v0,0x3
80101d44:  beq a1,v0,0x80101d5c
80101d48:  _li v0,0x6
80101d4c:  beq a1,v0,0x80101f9c
80101d50:  _clear v0
80101d54:  slti a0,zero,0x7f1
80101d58:  nop
80101d5c:  lw s0,0x0(a2)
80101d60:  nop
80101d64:  lbu v0,0x4(s0)
80101d68:  nop
80101d6c:  bne v0,a3,0x80101fc4
80101d70:  _clear v0
80101d74:  lw v0,0x0(s0)
80101d78:  nop
80101d7c:  andi v0,v0,0x4000
80101d80:  beq v0,zero,0x80101e20
80101d84:  _addiu a0,s2,0x80
80101d88:  jal 0x8001fe50
80101d8c:  _move a1,s0
80101d90:  lui v1,0x8006
80101d94:  lw v1,0x5310(v1)
80101d98:  nop
80101d9c:  addiu v1,v1,0x12c
80101da0:  ori v1,v1,0x1
80101da4:  sw v1,0xc(v0)
80101da8:  lw v0,0x0(s2)
80101dac:  nop
80101db0:  andi v0,v0,0x1
80101db4:  bne v0,zero,0x80101dcc
80101db8:  _lui v0,0x8006
80101dbc:  move a0,s2
80101dc0:  jal 0x80020890
80101dc4:  _li a1,0x3c
80101dc8:  lui v0,0x8006
80101dcc:  lw v1,0x59fc(v0)
80101dd0:  li v0,0x4
80101dd4:  sw v0,0x10(sp)
80101dd8:  li v0,0x3c
80101ddc:  sw v0,0x14(sp)
80101de0:  lw a1,0x58(v1)
80101de4:  addiu a0,s0,0x24
80101de8:  li a2,0x58b
80101dec:  sltiu a0,zero,0x6ee
80101df0:  li a3,0x58a
80101df4:  lw v1,0x0(s0)
80101df8:  lui a0,0x8006
80101dfc:  addiu a0,a0,0x5a18
80101e00:  li a1,0x9
80101e04:  move a2,s0
80101e08:  li v0,-0x4001
80101e0c:  and v1,v1,v0
80101e10:  jal 0x8002002c
80101e14:  _sw v1,0x0(a2)
80101e18:  slti a0,zero,0x7f1
80101e1c:  clear v0
80101e20:  jal 0x8001ff0c
80101e24:  _move a1,s0
80101e28:  lw v1,0xc(v0)
80101e2c:  nop
80101e30:  ori v1,v1,0x1
80101e34:  slti a0,zero,0x7f0
80101e38:  sw v1,0xc(v0)
80101e3c:  lw s0,0x80(s2)
80101e40:  nop
80101e44:  lw s1,0x0(s0)
80101e48:  nop
80101e4c:  beq s1,zero,0x80101f34
80101e50:  _clear s3
80101e54:  lw a0,0xc(s0)
80101e58:  nop
80101e5c:  andi v0,a0,0x1
80101e60:  beq v0,zero,0x80101f04
80101e64:  _nop
80101e68:  lw v0,0x8(s0)
80101e6c:  nop
80101e70:  lhu v0,0xc(v0)
80101e74:  nop
80101e78:  beq v0,zero,0x80101f04
80101e7c:  _lui v0,0x8006
80101e80:  lw v1,0x5310(v0)
80101e84:  li v0,-0x2
80101e88:  and v0,a0,v0
80101e8c:  sw v0,0xc(s0)
80101e90:  sltu v0,v0,v1
80101e94:  beq v0,zero,0x80101ee8
80101e98:  _li a1,-0x14
80101e9c:  lw a0,0x8(s0)
80101ea0:  lui a2,0x8010
80101ea4:  addiu a2,a2,0x110
80101ea8:  jal 0x8002c6fc
80101eac:  _li a3,0x1
80101eb0:  lb v0,0x5(s2)
80101eb4:  nop
80101eb8:  bne v0,zero,0x80101ee8
80101ebc:  _li s3,0x1
80101ec0:  jal 0x8004410c
80101ec4:  _nop
80101ec8:  lw v1,0x58(s2)
80101ecc:  sb v0,0x5(s2)
80101ed0:  lw a1,0x8(v1)
80101ed4:  sll v0,v0,0x18
80101ed8:  sra a0,v0,0x18
80101edc:  li a2,0x2
80101ee0:  jal 0x800443c8
80101ee4:  _clear a3
80101ee8:  lw a2,0x8(s0)
80101eec:  lui a0,0x8006
80101ef0:  addiu a0,a0,0x5a18
80101ef4:  jal 0x8002002c
80101ef8:  _li a1,0x9
80101efc:  slti a0,zero,0x7c9
80101f00:  move s0,s1
80101f04:  lw v1,0x8(s0)
80101f08:  nop
80101f0c:  lw v0,0x0(v1)
80101f10:  move a0,s0
80101f14:  ori v0,v0,0x4000
80101f18:  jal 0x80022c78
80101f1c:  _sw v0,0x0(v1)
80101f20:  move s0,s1
80101f24:  lw s1,0x0(s1)
80101f28:  nop
80101f2c:  bne s1,zero,0x80101e54
80101f30:  _nop
80101f34:  bne s3,zero,0x80101f58
80101f38:  _nop
80101f3c:  lb a0,0x5(s2)
80101f40:  nop
80101f44:  beq a0,zero,0x80101f7c
80101f48:  _nop
80101f4c:  jal 0x800441c8
80101f50:  _nop
80101f54:  sb zero,0x5(s2)
80101f58:  lb v0,0x5(s2)
80101f5c:  nop
80101f60:  beq v0,zero,0x80101f7c
80101f64:  _nop
80101f68:  jal 0x800446dc
80101f6c:  _addiu a0,s2,0x48
80101f70:  lb a0,0x5(s2)
80101f74:  jal 0x80044574
80101f78:  _move a1,v0
80101f7c:  lw v1,0x88(s2)
80101f80:  addiu v0,s2,0x80
80101f84:  beq v1,v0,0x80101fc0
80101f88:  _move a0,s2
80101f8c:  jal 0x80020890
80101f90:  _li a1,0x3c
80101f94:  slti a0,zero,0x7f1
80101f98:  clear v0
80101f9c:  slti a0,zero,0x7f1
80101fa0:  li v0,0x8c
80101fa4:  li v0,0x3
80101fa8:  sb v0,0x4(s2)
80101fac:  addiu v0,s2,0x84
80101fb0:  sw v0,0x80(s2)
80101fb4:  addiu v0,s2,0x80
80101fb8:  sw zero,0x4(v0)
80101fbc:  sw v0,0x8(v0)
80101fc0:  clear v0
80101fc4:  lw ra,0x28(sp)
80101fc8:  lw s3,0x24(sp)
80101fcc:  lw s2,0x20(sp)
80101fd0:  lw s1,0x1c(sp)
80101fd4:  lw s0,0x18(sp)
80101fd8:  jr ra
80101fdc:  _addiu sp,sp,0x30
