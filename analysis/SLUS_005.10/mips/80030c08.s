# addr: 0x80030c08  name: FUN_80030c08
80030c08:  addiu sp,sp,-0x20
80030c0c:  sw s0,0x10(sp)
80030c10:  move s0,a0
80030c14:  sw s2,0x18(sp)
80030c18:  addiu s2,s0,0xa4
80030c1c:  addiu a1,s0,0x24
80030c20:  clear a2
80030c24:  move a3,a2
80030c28:  sw ra,0x1c(sp)
80030c2c:  jal 0x8001d748
80030c30:  _sw s1,0x14(sp)
80030c34:  lw v1,0x28(s0)
80030c38:  nop
80030c3c:  subu v0,v0,v1
80030c40:  bgez v0,0x80030c4c
80030c44:  _addiu a0,s0,0x80
80030c48:  addiu v0,v0,0xff
80030c4c:  jal 0x80016a20
80030c50:  _sra s1,v0,0x8
80030c54:  move v1,v0
80030c58:  bgez v1,0x80030c64
80030c5c:  _nop
80030c60:  addiu v1,v1,0x7f
80030c64:  lh v0,0xa4(s0)
80030c68:  lh a0,0x1a(s0)
80030c6c:  sra v1,v1,0x7
80030c70:  sw v1,0x8c(s0)
80030c74:  sll v0,v0,0x6
80030c78:  sw v0,0x94(s0)
80030c7c:  lw v0,0x90(s0)
80030c80:  bgtz a0,0x80030c8c
80030c84:  _addiu v1,v0,0x200
80030c88:  addiu v1,v0,-0x200
80030c8c:  lh v0,0x16(s0)
80030c90:  sw v1,0x90(s0)
80030c94:  lw v1,0x98(s0)
80030c98:  bgez v0,0x80030ca4
80030c9c:  _addiu v0,v1,-0x200
80030ca0:  addiu v0,v1,0x200
80030ca4:  lh v1,0x14(s0)
80030ca8:  sw v0,0x98(s0)
80030cac:  lh v0,0x2(s2)
80030cb0:  nop
80030cb4:  mult v1,v0
80030cb8:  mflo a2
80030cbc:  bgez a2,0x80030cc8
80030cc0:  _nop
80030cc4:  addiu a2,a2,0xf
80030cc8:  lw v1,0x80(s0)
80030ccc:  lw a1,0x84(s0)
80030cd0:  bgez s1,0x80030cdc
80030cd4:  _move v0,s1
80030cd8:  subu v0,zero,v0
80030cdc:  mult s1,v0
80030ce0:  lw a0,0xd8(s0)
80030ce4:  li a3,0x800
80030ce8:  sra v0,a2,0x4
80030cec:  addu v1,v1,v0
80030cf0:  sll v0,a0,0x3
80030cf4:  subu v0,v0,a0
80030cf8:  sw v1,0x80(s0)
80030cfc:  sll v1,v0,0xa
80030d00:  mflo t0
80030d04:  slt v0,a3,t0
80030d08:  beq v0,zero,0x80030d14
80030d0c:  _addiu a1,a1,0x1c00
80030d10:  move a3,t0
80030d14:  div v1,a3
80030d18:  mflo v0
80030d1c:  lh v1,0x20(s0)
80030d20:  subu v0,a1,v0
80030d24:  sw v0,0x84(s0)
80030d28:  lh v0,0x2(s2)
80030d2c:  nop
80030d30:  mult v1,v0
80030d34:  mflo v1
80030d38:  bgez v1,0x80030d44
80030d3c:  _nop
80030d40:  addiu v1,v1,0xf
80030d44:  lw v0,0x88(s0)
80030d48:  move a0,s0
80030d4c:  sra v1,v1,0x4
80030d50:  addu v0,v0,v1
80030d54:  jal 0x80017324
80030d58:  _sw v0,0x88(s0)
80030d5c:  lw v0,0x90(s0)
80030d60:  li a0,0xf80
80030d64:  mult v0,a0
80030d68:  mflo v1
80030d6c:  sra v0,v1,0xc
80030d70:  addiu v1,s0,0x90
80030d74:  sw v0,0x90(s0)
80030d78:  lw v0,0x4(v1)
80030d7c:  nop
80030d80:  mult v0,a0
80030d84:  lw v0,0x8(v1)
80030d88:  mflo t0
80030d8c:  nop
80030d90:  nop
80030d94:  mult v0,a0
80030d98:  sra v0,t0,0xc
80030d9c:  sw v0,0x4(v1)
80030da0:  mflo a0
80030da4:  sra v0,a0,0xc
80030da8:  sw v0,0x8(v1)
80030dac:  lw v1,0x80(s0)
80030db0:  nop
80030db4:  bgez v1,0x80030dc0
80030db8:  _move v0,v1
80030dbc:  addiu v0,v1,0x3f
80030dc0:  lw a0,0x84(s0)
80030dc4:  sra v0,v0,0x6
80030dc8:  subu v0,v1,v0
80030dcc:  sw v0,0x80(s0)
80030dd0:  bgez a0,0x80030ddc
80030dd4:  _move v0,a0
80030dd8:  addiu v0,a0,0x3f
80030ddc:  lw v1,0x88(s0)
80030de0:  sra v0,v0,0x6
80030de4:  subu v0,a0,v0
80030de8:  sw v0,0x84(s0)
80030dec:  bgez v1,0x80030df8
80030df0:  _move v0,v1
80030df4:  addiu v0,v1,0x3f
80030df8:  sra v0,v0,0x6
80030dfc:  subu v0,v1,v0
80030e00:  sw v0,0x88(s0)
80030e04:  clear a1
80030e08:  move a2,s0
80030e0c:  lw a0,0x110(a2)
80030e10:  nop
80030e14:  beq a0,zero,0x80030e30
80030e18:  _nop
80030e1c:  lh v0,0x6(a0)
80030e20:  lhu v1,0x6(a0)
80030e24:  beq v0,zero,0x80030e30
80030e28:  _addiu v0,v1,-0x1
80030e2c:  sh v0,0x6(a0)
80030e30:  addiu a1,a1,0x1
80030e34:  slti v0,a1,0x3
80030e38:  bne v0,zero,0x80030e0c
80030e3c:  _addiu a2,a2,0x4
80030e40:  clear a1
80030e44:  move v1,s0
80030e48:  lhu v0,0x11c(v1)
80030e4c:  nop
80030e50:  beq v0,zero,0x80030e5c
80030e54:  _addiu v0,v0,-0x1
80030e58:  sh v0,0x11c(v1)
80030e5c:  addiu a1,a1,0x1
80030e60:  slti v0,a1,0x3
80030e64:  bne v0,zero,0x80030e48
80030e68:  _addiu v1,v1,0x2
80030e6c:  lw v0,0x0(s0)
80030e70:  lui v1,0x80
80030e74:  and v0,v0,v1
80030e78:  bne v0,zero,0x80030f1c
80030e7c:  _nop
80030e80:  lhu v0,0x120(s0)
80030e84:  nop
80030e88:  beq v0,zero,0x80030f04
80030e8c:  _nop
80030e90:  lw v0,0x24(s0)
80030e94:  lw a0,0x48(s0)
80030e98:  nop
80030e9c:  subu v0,v0,a0
80030ea0:  bgez v0,0x80030eac
80030ea4:  _nop
80030ea8:  addiu v0,v0,0x1f
80030eac:  lw v1,0x28(s0)
80030eb0:  lw a1,0x4c(s0)
80030eb4:  sra v0,v0,0x5
80030eb8:  addu v0,a0,v0
80030ebc:  sw v0,0x48(s0)
80030ec0:  subu v0,v1,a1
80030ec4:  bgez v0,0x80030ed0
80030ec8:  _nop
80030ecc:  addiu v0,v0,0x1f
80030ed0:  lw v1,0x2c(s0)
80030ed4:  lw a0,0x50(s0)
80030ed8:  sra v0,v0,0x5
80030edc:  addu v0,a1,v0
80030ee0:  sw v0,0x4c(s0)
80030ee4:  subu v0,v1,a0
80030ee8:  bgez v0,0x80030ef4
80030eec:  _nop
80030ef0:  addiu v0,v0,0x1f
80030ef4:  sra v0,v0,0x5
80030ef8:  addu v0,a0,v0
80030efc:  j 0x80030f1c
80030f00:  _sw v0,0x50(s0)
80030f04:  lw t1,0x24(s0)
80030f08:  lw t2,0x28(s0)
80030f0c:  lw t3,0x2c(s0)
80030f10:  sw t1,0x48(s0)
80030f14:  sw t2,0x4c(s0)
80030f18:  sw t3,0x50(s0)
80030f1c:  lw ra,0x1c(sp)
80030f20:  lw s2,0x18(sp)
80030f24:  lw s1,0x14(sp)
80030f28:  lw s0,0x10(sp)
80030f2c:  jr ra
80030f30:  _addiu sp,sp,0x20
