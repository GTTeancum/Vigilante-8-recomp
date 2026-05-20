# addr: 0x80015c68  name: FUN_80015c68
80015c68:  addiu sp,sp,-0x28
80015c6c:  sw s2,0x18(sp)
80015c70:  move s2,a0
80015c74:  sw s3,0x1c(sp)
80015c78:  move s3,a2
80015c7c:  move a0,a1
80015c80:  sw ra,0x24(sp)
80015c84:  sw s4,0x20(sp)
80015c88:  sw s1,0x14(sp)
80015c8c:  jal 0x8001570c
80015c90:  _sw s0,0x10(sp)
80015c94:  sw zero,0x10(s2)
80015c98:  beq s3,zero,0x80015e10
80015c9c:  _sw zero,0x8(s2)
80015ca0:  li s4,0x3b
80015ca4:  jal 0x800156d4
80015ca8:  _nop
80015cac:  move s1,v0
80015cb0:  lbu v0,0x0(s1)
80015cb4:  nop
80015cb8:  beq v0,zero,0x80015e04
80015cbc:  _nop
80015cc0:  lbu v0,0x19(s1)
80015cc4:  nop
80015cc8:  andi v0,v0,0x2
80015ccc:  beq v0,zero,0x80015d58
80015cd0:  _clear a0
80015cd4:  lbu v0,0x21(s1)
80015cd8:  nop
80015cdc:  sltiu v0,v0,0x2
80015ce0:  bne v0,zero,0x80015de8
80015ce4:  _nop
80015ce8:  jal 0x800116f4
80015cec:  _li a0,0x514
80015cf0:  move s0,v0
80015cf4:  move a0,s0
80015cf8:  li a1,0x20
80015cfc:  jal 0x80044efc
80015d00:  _li a2,0x8
80015d04:  lbu v1,0x20(s1)
80015d08:  li a2,0x8
80015d0c:  slt v0,v1,a2
80015d10:  beq v0,zero,0x80015d1c
80015d14:  _move a0,s0
80015d18:  move a2,v1
80015d1c:  jal 0x80044c44
80015d20:  _addiu a1,s1,0x21
80015d24:  lwl v0,0x5(s1)
80015d28:  lwr v0,0x2(s1)
80015d2c:  nop
80015d30:  sw v0,0x20(s0)
80015d34:  lwl v0,0xd(s1)
80015d38:  lwr v0,0xa(s1)
80015d3c:  nop
80015d40:  sw v0,0x24(s0)
80015d44:  lw v0,0x8(s2)
80015d48:  nop
80015d4c:  sw v0,0xc(s0)
80015d50:  j 0x80015de8
80015d54:  _sw s0,0x8(s2)
80015d58:  lw v1,0x10(s2)
80015d5c:  nop
80015d60:  sll v0,v1,0x2
80015d64:  addu v0,v0,v1
80015d68:  sll v0,v0,0x2
80015d6c:  addiu v0,v0,0x14
80015d70:  addu a1,s2,v0
80015d74:  addiu v1,v1,0x1
80015d78:  sw v1,0x10(s2)
80015d7c:  addu v0,s1,a0
80015d80:  lbu v1,0x21(v0)
80015d84:  nop
80015d88:  beq v1,s4,0x80015da4
80015d8c:  _addu v0,a1,a0
80015d90:  sb v1,0x0(v0)
80015d94:  addiu a0,a0,0x1
80015d98:  sltiu v0,a0,0xc
80015d9c:  bne v0,zero,0x80015d80
80015da0:  _addu v0,s1,a0
80015da4:  sltiu v0,a0,0xc
80015da8:  beq v0,zero,0x80015dc8
80015dac:  _addu v0,a1,a0
80015db0:  li v1,0x20
80015db4:  sb v1,0x0(v0)
80015db8:  addiu a0,a0,0x1
80015dbc:  sltiu v0,a0,0xc
80015dc0:  bne v0,zero,0x80015db4
80015dc4:  _addu v0,a1,a0
80015dc8:  lwl v0,0x5(s1)
80015dcc:  lwr v0,0x2(s1)
80015dd0:  nop
80015dd4:  sw v0,0xc(a1)
80015dd8:  lwl v0,0xd(s1)
80015ddc:  lwr v0,0xa(s1)
80015de0:  nop
80015de4:  sw v0,0x10(a1)
80015de8:  lbu v0,0x0(s1)
80015dec:  nop
80015df0:  addu s1,s1,v0
80015df4:  lbu v0,0x0(s1)
80015df8:  nop
80015dfc:  bne v0,zero,0x80015cc0
80015e00:  _nop
80015e04:  addiu s3,s3,-0x800
80015e08:  bne s3,zero,0x80015ca4
80015e0c:  _nop
80015e10:  jal 0x80015798
80015e14:  _nop
80015e18:  lw v0,0x10(s2)
80015e1c:  move a0,s2
80015e20:  sll a1,v0,0x2
80015e24:  addu a1,a1,v0
80015e28:  sll a1,a1,0x2
80015e2c:  jal 0x80045134
80015e30:  _addiu a1,a1,0x14
80015e34:  move s2,v0
80015e38:  lw s0,0x8(s2)
80015e3c:  nop
80015e40:  beq s0,zero,0x80015e68
80015e44:  _nop
80015e48:  lw a1,0x20(s0)
80015e4c:  lw a2,0x24(s0)
80015e50:  jal 0x80015c68
80015e54:  _move a0,s0
80015e58:  lw s0,0xc(s0)
80015e5c:  nop
80015e60:  bne s0,zero,0x80015e48
80015e64:  _nop
80015e68:  lw ra,0x24(sp)
80015e6c:  lw s4,0x20(sp)
80015e70:  lw s3,0x1c(sp)
80015e74:  move v0,s2
80015e78:  lw s2,0x18(sp)
80015e7c:  lw s1,0x14(sp)
80015e80:  lw s0,0x10(sp)
80015e84:  jr ra
80015e88:  _addiu sp,sp,0x28
