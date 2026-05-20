# addr: 0x800289d8  name: FUN_800289d8
800289d8:  addiu sp,sp,-0x70
800289dc:  move t0,a0
800289e0:  li v0,0x1
800289e4:  sllv v0,v0,t0
800289e8:  sw s5,0x5c(sp)
800289ec:  subu s5,zero,v0
800289f0:  sw v0,0x18(sp)
800289f4:  li v0,0x2
800289f8:  sw ra,0x6c(sp)
800289fc:  sw s8,0x68(sp)
80028a00:  sw s7,0x64(sp)
80028a04:  sw s6,0x60(sp)
80028a08:  sw s4,0x58(sp)
80028a0c:  sw s3,0x54(sp)
80028a10:  sw s2,0x50(sp)
80028a14:  sw s1,0x4c(sp)
80028a18:  sw s0,0x48(sp)
80028a1c:  sw a1,0x74(sp)
80028a20:  beq t0,v0,0x80028a30
80028a24:  _sw s5,0x1c(sp)
80028a28:  sll t1,s5,0x1
80028a2c:  sw t1,0x1c(sp)
80028a30:  lw t2,0x74(sp)
80028a34:  nop
80028a38:  lh v1,0x6(t2)
80028a3c:  lh a1,0x2(t2)
80028a40:  lui s3,0x7fff
80028a44:  ori s3,s3,0xffff
80028a48:  lui t1,0x8000
80028a4c:  ori t1,t1,0x1
80028a50:  sw t1,0x20(sp)
80028a54:  slt v0,a1,v1
80028a58:  beq v0,zero,0x80028a64
80028a5c:  _move a0,v1
80028a60:  move a0,a1
80028a64:  sw a0,0x24(sp)
80028a68:  slt a0,v1,a1
80028a6c:  beq a0,zero,0x80028a78
80028a70:  _nop
80028a74:  move v1,a1
80028a78:  lw t2,0x74(sp)
80028a7c:  nop
80028a80:  lh v0,0x0(t2)
80028a84:  sw v1,0x28(sp)
80028a88:  sw zero,0x30(sp)
80028a8c:  sw zero,0x34(sp)
80028a90:  lh v1,0x4(t2)
80028a94:  clear s6
80028a98:  sll a0,a0,0x1
80028a9c:  move a1,t2
80028aa0:  sw zero,0x38(sp)
80028aa4:  sw zero,0x3c(sp)
80028aa8:  slt v1,v1,v0
80028aac:  or a0,a0,v1
80028ab0:  sw a0,0x2c(sp)
80028ab4:  lh v1,0x2(a1)
80028ab8:  nop
80028abc:  slt v0,v1,s3
80028ac0:  beq v0,zero,0x80028ad0
80028ac4:  _nop
80028ac8:  move s3,v1
80028acc:  move s8,s6
80028ad0:  lw t1,0x20(sp)
80028ad4:  nop
80028ad8:  slt v0,t1,v1
80028adc:  beq v0,zero,0x80028ae8
80028ae0:  _nop
80028ae4:  sw v1,0x20(sp)
80028ae8:  addiu s6,s6,0x1
80028aec:  slti v0,s6,0x4
80028af0:  bne v0,zero,0x80028ab4
80028af4:  _addiu a1,a1,0x4
80028af8:  lw t2,0x74(sp)
80028afc:  move s6,s8
80028b00:  sll v0,s6,0x2
80028b04:  addu v0,v0,t2
80028b08:  lh v0,0x0(v0)
80028b0c:  nop
80028b10:  sll a2,v0,0x10
80028b14:  beq t0,zero,0x80028b44
80028b18:  _move a3,a2
80028b1c:  lh v1,0x2(t2)
80028b20:  lh v0,0x6(t2)
80028b24:  nop
80028b28:  bne v1,v0,0x80028b44
80028b2c:  _nop
80028b30:  lw t1,0x20(sp)
80028b34:  lw t2,0x18(sp)
80028b38:  nop
80028b3c:  subu t1,t1,t2
80028b40:  sw t1,0x20(sp)
80028b44:  lw t1,0x20(sp)
80028b48:  nop
80028b4c:  slt v0,s3,t1
80028b50:  beq v0,zero,0x80029040
80028b54:  _nop
80028b58:  lw t2,0x34(sp)
80028b5c:  nop
80028b60:  bgtz t2,0x80028bd8
80028b64:  _nop
80028b68:  lw t1,0x74(sp)
80028b6c:  sll v0,s8,0x2
80028b70:  addu v0,v0,t1
80028b74:  lh v1,0x0(v0)
80028b78:  addiu v0,s8,0x1
80028b7c:  andi s8,v0,0x3
80028b80:  sll v0,s8,0x2
80028b84:  addu a1,v0,t1
80028b88:  lh v0,0x2(a1)
80028b8c:  clear a0
80028b90:  li t2,0x2
80028b94:  sll s4,v1,0x10
80028b98:  beq t0,t2,0x80028ba8
80028b9c:  _subu v1,v0,s3
80028ba0:  xori v0,s8,0x3
80028ba4:  sltiu a0,v0,0x1
80028ba8:  addu v0,t0,a0
80028bac:  srav v1,v1,v0
80028bb0:  blez v1,0x80028b68
80028bb4:  _sw v1,0x34(sp)
80028bb8:  lh v0,0x0(a1)
80028bbc:  nop
80028bc0:  sll v0,v0,0x10
80028bc4:  subu v0,v0,s4
80028bc8:  div v0,v1
80028bcc:  mflo t1
80028bd0:  nop
80028bd4:  sw t1,0x30(sp)
80028bd8:  lw t2,0x3c(sp)
80028bdc:  nop
80028be0:  bgtz t2,0x80028c58
80028be4:  _nop
80028be8:  lw t1,0x74(sp)
80028bec:  sll v0,s6,0x2
80028bf0:  addu v0,v0,t1
80028bf4:  lh v1,0x0(v0)
80028bf8:  addiu v0,s6,0x3
80028bfc:  andi s6,v0,0x3
80028c00:  sll v0,s6,0x2
80028c04:  addu a1,v0,t1
80028c08:  lh v0,0x2(a1)
80028c0c:  clear a0
80028c10:  li t2,0x2
80028c14:  sll s7,v1,0x10
80028c18:  beq t0,t2,0x80028c28
80028c1c:  _subu v1,v0,s3
80028c20:  xor v0,s6,t2
80028c24:  sltiu a0,v0,0x1
80028c28:  addu v0,t0,a0
80028c2c:  srav v1,v1,v0
80028c30:  blez v1,0x80028be8
80028c34:  _sw v1,0x3c(sp)
80028c38:  lh v0,0x0(a1)
80028c3c:  nop
80028c40:  sll v0,v0,0x10
80028c44:  subu v0,v0,s7
80028c48:  div v0,v1
80028c4c:  mflo t1
80028c50:  nop
80028c54:  sw t1,0x38(sp)
80028c58:  beq t0,zero,0x80028f54
80028c5c:  _li v0,0x3
80028c60:  lw t2,0x24(sp)
80028c64:  nop
80028c68:  slt v0,s3,t2
80028c6c:  bne v0,zero,0x80028f54
80028c70:  _li v0,0x3
80028c74:  lw t1,0x28(sp)
80028c78:  nop
80028c7c:  slt v0,t1,s3
80028c80:  bne v0,zero,0x80028f54
80028c84:  _li v0,0x3
80028c88:  lw t2,0x2c(sp)
80028c8c:  li v0,0x1
80028c90:  beq t2,v0,0x80028d5c
80028c94:  _slti v0,t2,0x2
80028c98:  beq v0,zero,0x80028cb0
80028c9c:  _nop
80028ca0:  beq t2,zero,0x80028cd0
80028ca4:  _move a3,s4
80028ca8:  j 0x80028fb0
80028cac:  _li t2,0x2
80028cb0:  lw t1,0x2c(sp)
80028cb4:  li t2,0x2
80028cb8:  beq t1,t2,0x80028dec
80028cbc:  _li v0,0x3
80028cc0:  beq t1,v0,0x80028ea4
80028cc4:  _move a3,s4
80028cc8:  j 0x80028fac
80028ccc:  _nop
80028cd0:  lw t1,0x30(sp)
80028cd4:  sra v0,s4,0x10
80028cd8:  and s2,v0,s5
80028cdc:  addu s0,s4,t1
80028ce0:  slt v0,s7,s0
80028ce4:  beq v0,zero,0x80028cf0
80028ce8:  _move v1,s0
80028cec:  move v1,s7
80028cf0:  sra v0,v1,0x10
80028cf4:  and s1,v0,s5
80028cf8:  li v0,0x5
80028cfc:  move a0,t0
80028d00:  move a1,s2
80028d04:  move a2,s1
80028d08:  move a3,s3
80028d0c:  sw v0,0x10(sp)
80028d10:  jal 0x800288fc
80028d14:  _sw t0,0x40(sp)
80028d18:  sra v0,s0,0x10
80028d1c:  and a1,v0,s5
80028d20:  li t2,0x2
80028d24:  lw t0,0x40(sp)
80028d28:  beq s6,t2,0x80028d38
80028d2c:  _sra a2,s7,0x10
80028d30:  j 0x80028d44
80028d34:  _and a2,a2,s5
80028d38:  lw t1,0x1c(sp)
80028d3c:  nop
80028d40:  and a2,a2,t1
80028d44:  slt v0,s2,s1
80028d48:  xori v0,v0,0x1
80028d4c:  move a0,t0
80028d50:  move a3,s3
80028d54:  j 0x80028f9c
80028d58:  _sw v0,0x10(sp)
80028d5c:  sra v0,s4,0x10
80028d60:  and s2,v0,s5
80028d64:  move v1,a3
80028d68:  slt v0,s7,v1
80028d6c:  beq v0,zero,0x80028d7c
80028d70:  _sra v0,v1,0x10
80028d74:  move v1,s7
80028d78:  sra v0,v1,0x10
80028d7c:  and s1,v0,s5
80028d80:  li v0,0x9
80028d84:  move a0,t0
80028d88:  move a1,s2
80028d8c:  move a2,s1
80028d90:  move a3,s3
80028d94:  sw v0,0x10(sp)
80028d98:  jal 0x800288fc
80028d9c:  _sw t0,0x40(sp)
80028da0:  lw t2,0x30(sp)
80028da4:  sra a2,s7,0x10
80028da8:  li t1,0x2
80028dac:  lw t0,0x40(sp)
80028db0:  subu v0,s4,t2
80028db4:  sra v0,v0,0x10
80028db8:  beq s6,t1,0x80028dc8
80028dbc:  _and a1,v0,s5
80028dc0:  j 0x80028dd4
80028dc4:  _and a2,a2,s5
80028dc8:  lw t2,0x1c(sp)
80028dcc:  nop
80028dd0:  and a2,a2,t2
80028dd4:  slt v0,s2,s1
80028dd8:  xori v0,v0,0x1
80028ddc:  move a0,t0
80028de0:  move a3,s3
80028de4:  j 0x80028f9c
80028de8:  _sw v0,0x10(sp)
80028dec:  lw t1,0x38(sp)
80028df0:  nop
80028df4:  addu v1,s7,t1
80028df8:  slt v0,v1,s4
80028dfc:  beq v0,zero,0x80028e0c
80028e00:  _sra v0,v1,0x10
80028e04:  move v1,s4
80028e08:  sra v0,v1,0x10
80028e0c:  and s2,v0,s5
80028e10:  sra v0,s7,0x10
80028e14:  and s1,v0,s5
80028e18:  li v0,0x6
80028e1c:  move a0,t0
80028e20:  move a1,s2
80028e24:  move a2,s1
80028e28:  move a3,s3
80028e2c:  sw v0,0x10(sp)
80028e30:  jal 0x800288fc
80028e34:  _sw t0,0x40(sp)
80028e38:  li v0,0x3
80028e3c:  lw t0,0x40(sp)
80028e40:  beq s8,v0,0x80028e50
80028e44:  _sra a1,s4,0x10
80028e48:  j 0x80028e5c
80028e4c:  _and a1,a1,s5
80028e50:  lw t2,0x1c(sp)
80028e54:  nop
80028e58:  and a1,a1,t2
80028e5c:  lw t1,0x38(sp)
80028e60:  nop
80028e64:  addu v1,s7,t1
80028e68:  slt v0,v1,s4
80028e6c:  beq v0,zero,0x80028e7c
80028e70:  _sra v0,v1,0x10
80028e74:  move v1,s4
80028e78:  sra v0,v1,0x10
80028e7c:  and a2,v0,s5
80028e80:  slt v0,s2,s1
80028e84:  bne v0,zero,0x80028e94
80028e88:  _li t2,0x2
80028e8c:  j 0x80028e98
80028e90:  _sw t2,0x10(sp)
80028e94:  sw zero,0x10(sp)
80028e98:  move a0,t0
80028e9c:  j 0x80028f9c
80028ea0:  _move a3,s3
80028ea4:  move v1,a2
80028ea8:  slt v0,v1,s4
80028eac:  beq v0,zero,0x80028ebc
80028eb0:  _sra v0,v1,0x10
80028eb4:  move v1,s4
80028eb8:  sra v0,v1,0x10
80028ebc:  and s2,v0,s5
80028ec0:  sra v0,s7,0x10
80028ec4:  and s1,v0,s5
80028ec8:  li v0,0xa
80028ecc:  move a0,t0
80028ed0:  move a1,s2
80028ed4:  move a2,s1
80028ed8:  move a3,s3
80028edc:  sw v0,0x10(sp)
80028ee0:  jal 0x800288fc
80028ee4:  _sw t0,0x40(sp)
80028ee8:  lw t1,0x2c(sp)
80028eec:  lw t0,0x40(sp)
80028ef0:  beq s8,t1,0x80028f00
80028ef4:  _sra a1,s4,0x10
80028ef8:  j 0x80028f0c
80028efc:  _and a1,a1,s5
80028f00:  lw t2,0x1c(sp)
80028f04:  nop
80028f08:  and a1,a1,t2
80028f0c:  lw t1,0x38(sp)
80028f10:  nop
80028f14:  subu v1,s7,t1
80028f18:  slt v0,v1,s4
80028f1c:  beq v0,zero,0x80028f2c
80028f20:  _sra v0,v1,0x10
80028f24:  move v1,s4
80028f28:  sra v0,v1,0x10
80028f2c:  and a2,v0,s5
80028f30:  slt v0,s2,s1
80028f34:  bne v0,zero,0x80028f44
80028f38:  _li t2,0x2
80028f3c:  j 0x80028f48
80028f40:  _sw t2,0x10(sp)
80028f44:  sw zero,0x10(sp)
80028f48:  move a0,t0
80028f4c:  j 0x80028f9c
80028f50:  _move a3,s3
80028f54:  beq s8,v0,0x80028f64
80028f58:  _sra a1,s4,0x10
80028f5c:  j 0x80028f70
80028f60:  _and a1,a1,s5
80028f64:  lw t1,0x1c(sp)
80028f68:  nop
80028f6c:  and a1,a1,t1
80028f70:  li t2,0x2
80028f74:  beq s6,t2,0x80028f84
80028f78:  _sra a2,s7,0x10
80028f7c:  j 0x80028f90
80028f80:  _and a2,a2,s5
80028f84:  lw t1,0x1c(sp)
80028f88:  nop
80028f8c:  and a2,a2,t1
80028f90:  move a0,t0
80028f94:  move a3,s3
80028f98:  sw zero,0x10(sp)
80028f9c:  jal 0x800288fc
80028fa0:  _sw t0,0x40(sp)
80028fa4:  lw t0,0x40(sp)
80028fa8:  move a3,s4
80028fac:  li t2,0x2
80028fb0:  beq t0,t2,0x80028fd8
80028fb4:  _move a2,s7
80028fb8:  li v0,0x3
80028fbc:  bne s8,v0,0x80028fd8
80028fc0:  _nop
80028fc4:  lw t1,0x18(sp)
80028fc8:  nop
80028fcc:  and v0,s3,t1
80028fd0:  beq v0,zero,0x80028ff0
80028fd4:  _nop
80028fd8:  lw t2,0x30(sp)
80028fdc:  lw t1,0x34(sp)
80028fe0:  addu s4,s4,t2
80028fe4:  addiu t1,t1,-0x1
80028fe8:  sw t1,0x34(sp)
80028fec:  li t2,0x2
80028ff0:  beq t0,t2,0x80029014
80028ff4:  _nop
80028ff8:  bne s6,t2,0x80029014
80028ffc:  _nop
80029000:  lw t1,0x18(sp)
80029004:  nop
80029008:  and v0,s3,t1
8002900c:  beq v0,zero,0x80029028
80029010:  _nop
80029014:  lw t2,0x38(sp)
80029018:  lw t1,0x3c(sp)
8002901c:  addu s7,s7,t2
80029020:  addiu t1,t1,-0x1
80029024:  sw t1,0x3c(sp)
80029028:  lw t2,0x18(sp)
8002902c:  lw t1,0x20(sp)
80029030:  addu s3,s3,t2
80029034:  slt v0,s3,t1
80029038:  bne v0,zero,0x80028b58
8002903c:  _nop
80029040:  beq t0,zero,0x80029084
80029044:  _nop
80029048:  lw t2,0x74(sp)
8002904c:  nop
80029050:  lh v1,0x2(t2)
80029054:  lh v0,0x6(t2)
80029058:  nop
8002905c:  bne v1,v0,0x80029084
80029060:  _li v0,0x4
80029064:  sw v0,0x10(sp)
80029068:  move a0,t0
8002906c:  sra a1,s4,0x10
80029070:  and a1,a1,s5
80029074:  sra a2,s7,0x10
80029078:  and a2,a2,s5
8002907c:  jal 0x800288fc
80029080:  _move a3,s3
80029084:  lw ra,0x6c(sp)
80029088:  lw s8,0x68(sp)
8002908c:  lw s7,0x64(sp)
80029090:  lw s6,0x60(sp)
80029094:  lw s5,0x5c(sp)
80029098:  lw s4,0x58(sp)
8002909c:  lw s3,0x54(sp)
800290a0:  lw s2,0x50(sp)
800290a4:  lw s1,0x4c(sp)
800290a8:  lw s0,0x48(sp)
800290ac:  jr ra
800290b0:  _addiu sp,sp,0x70
