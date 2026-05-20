# addr: 0x80025bc0  name: FUN_80025bc0
80025bc0:  addiu sp,sp,-0x70
80025bc4:  sw s2,0x50(sp)
80025bc8:  lui s2,0x1f80
80025bcc:  lw s2,0x1c(s2)
80025bd0:  sw s5,0x5c(sp)
80025bd4:  move s5,a0
80025bd8:  sw s8,0x68(sp)
80025bdc:  move s8,a1
80025be0:  sw s7,0x64(sp)
80025be4:  li s7,0x90
80025be8:  sw ra,0x6c(sp)
80025bec:  sw s6,0x60(sp)
80025bf0:  sw s4,0x58(sp)
80025bf4:  sw s3,0x54(sp)
80025bf8:  sw s1,0x4c(sp)
80025bfc:  sw s0,0x48(sp)
80025c00:  sw a2,0x78(sp)
80025c04:  sw a3,0x7c(sp)
80025c08:  gte_stSZ1 s3
80025c0c:  nop
80025c10:  sw s3,0x40(sp)
80025c14:  gte_stSZ2 s4
80025c18:  nop
80025c1c:  move a0,s4
80025c20:  gte_stSZ3 s1
80025c24:  nop
80025c28:  move s6,s1
80025c2c:  strgb3_gt3 s5
80025c38:  slt v0,s3,s7
80025c3c:  beq v0,zero,0x80025dd4
80025c40:  _slt v0,s4,s7
80025c44:  beq v0,zero,0x80025cbc
80025c48:  _slt v0,s1,s7
80025c4c:  bne v0,zero,0x8002620c
80025c50:  _subu v1,s7,s3
80025c54:  sll v1,v1,0xc
80025c58:  subu v0,s1,s3
80025c5c:  div v1,v0
80025c60:  mflo v1
80025c64:  lw t0,0x0(a3)
80025c68:  lw t1,0x4(a3)
80025c6c:  addiu a0,sp,0x20
80025c70:  move a2,a3
80025c74:  addiu a3,s5,0xc
80025c78:  addiu s0,s5,0x24
80025c7c:  sw t0,0x30(sp)
80025c80:  sw t1,0x34(sp)
80025c84:  sw a3,0x10(sp)
80025c88:  sw s0,0x14(sp)
80025c8c:  jal 0x80025b20
80025c90:  _sw v1,0x18(sp)
80025c94:  subu v1,s7,s4
80025c98:  sll v1,v1,0xc
80025c9c:  subu v0,s1,s4
80025ca0:  div v1,v0
80025ca4:  mflo v1
80025ca8:  lw a1,0x78(sp)
80025cac:  lw a2,0x7c(sp)
80025cb0:  addiu a0,sp,0x28
80025cb4:  j 0x80025e50
80025cb8:  _addiu a3,s5,0x18
80025cbc:  slti v0,s1,0x90
80025cc0:  beq v0,zero,0x80025d38
80025cc4:  _subu v1,s7,s3
80025cc8:  sll v1,v1,0xc
80025ccc:  subu v0,s4,s3
80025cd0:  div v1,v0
80025cd4:  mflo v1
80025cd8:  lw t2,0x78(sp)
80025cdc:  nop
80025ce0:  lw t0,0x0(t2)
80025ce4:  lw t1,0x4(t2)
80025ce8:  addiu a0,sp,0x20
80025cec:  move a1,s8
80025cf0:  addiu a3,s5,0xc
80025cf4:  addiu s0,s5,0x18
80025cf8:  move a2,t2
80025cfc:  sw t0,0x28(sp)
80025d00:  sw t1,0x2c(sp)
80025d04:  sw a3,0x10(sp)
80025d08:  sw s0,0x14(sp)
80025d0c:  jal 0x80025b20
80025d10:  _sw v1,0x18(sp)
80025d14:  subu v1,s7,s1
80025d18:  sll v1,v1,0xc
80025d1c:  subu v0,s4,s1
80025d20:  div v1,v0
80025d24:  mflo v1
80025d28:  lw a1,0x7c(sp)
80025d2c:  lw a2,0x78(sp)
80025d30:  j 0x80025e4c
80025d34:  _addiu a0,sp,0x30
80025d38:  subu s0,s7,s3
80025d3c:  sll s0,s0,0xc
80025d40:  subu t0,s1,s3
80025d44:  lw t2,0x7c(sp)
80025d48:  div s0,t0
80025d4c:  mflo t0
80025d50:  lw v0,0x0(t2)
80025d54:  lw v1,0x4(t2)
80025d58:  sw v0,0x38(sp)
80025d5c:  sw v1,0x3c(sp)
80025d60:  lhu v0,0x24(s5)
80025d64:  nop
80025d68:  sh v0,0x30(s2)
80025d6c:  lw v0,0x1c(s5)
80025d70:  lw a2,0x7c(sp)
80025d74:  addiu a0,sp,0x30
80025d78:  move a1,s8
80025d7c:  addiu a3,s2,0x24
80025d80:  addiu s1,s5,0xc
80025d84:  sw v0,0x28(s2)
80025d88:  addiu v0,s5,0x24
80025d8c:  sw s1,0x10(sp)
80025d90:  sw v0,0x14(sp)
80025d94:  jal 0x80025b20
80025d98:  _sw t0,0x18(sp)
80025d9c:  subu v0,s4,s3
80025da0:  div s0,v0
80025da4:  mflo s0
80025da8:  lw a2,0x78(sp)
80025dac:  addiu a0,sp,0x20
80025db0:  move a1,s8
80025db4:  addiu a3,s2,0xc
80025db8:  addiu v0,s5,0x18
80025dbc:  sw s1,0x10(sp)
80025dc0:  sw v0,0x14(sp)
80025dc4:  jal 0x80025b20
80025dc8:  _sw s0,0x18(sp)
80025dcc:  j 0x80025fd4
80025dd0:  _nop
80025dd4:  slti v0,s4,0x90
80025dd8:  beq v0,zero,0x80025f34
80025ddc:  _slti v0,s1,0x90
80025de0:  beq v0,zero,0x80025e68
80025de4:  _subu v1,s7,s4
80025de8:  sll v1,v1,0xc
80025dec:  subu v0,s3,s4
80025df0:  div v1,v0
80025df4:  mflo v1
80025df8:  lw t0,0x0(s8)
80025dfc:  lw t1,0x4(s8)
80025e00:  lw a1,0x78(sp)
80025e04:  addiu a0,sp,0x28
80025e08:  move a2,s8
80025e0c:  addiu a3,s5,0x18
80025e10:  addiu s0,s5,0xc
80025e14:  sw t0,0x20(sp)
80025e18:  sw t1,0x24(sp)
80025e1c:  sw a3,0x10(sp)
80025e20:  sw s0,0x14(sp)
80025e24:  jal 0x80025b20
80025e28:  _sw v1,0x18(sp)
80025e2c:  subu v1,s7,s1
80025e30:  sll v1,v1,0xc
80025e34:  subu v0,s3,s1
80025e38:  div v1,v0
80025e3c:  mflo v1
80025e40:  lw a1,0x7c(sp)
80025e44:  addiu a0,sp,0x30
80025e48:  move a2,s8
80025e4c:  addiu a3,s5,0x24
80025e50:  sw a3,0x10(sp)
80025e54:  sw s0,0x14(sp)
80025e58:  jal 0x80025b20
80025e5c:  _sw v1,0x18(sp)
80025e60:  j 0x80026004
80025e64:  _nop
80025e68:  subu v1,s7,s1
80025e6c:  sll v1,v1,0xc
80025e70:  subu v0,s4,s1
80025e74:  div v1,v0
80025e78:  mflo v1
80025e7c:  lw a1,0x7c(sp)
80025e80:  lw a2,0x78(sp)
80025e84:  addiu a0,sp,0x38
80025e88:  addiu a3,s2,0x30
80025e8c:  addiu v0,s5,0x24
80025e90:  addiu s0,s5,0x18
80025e94:  sw v0,0x10(sp)
80025e98:  sw s0,0x14(sp)
80025e9c:  jal 0x80025b20
80025ea0:  _sw v1,0x18(sp)
80025ea4:  subu v1,s7,s4
80025ea8:  sll v1,v1,0xc
80025eac:  subu v0,s3,s4
80025eb0:  div v1,v0
80025eb4:  mflo v1
80025eb8:  lw a1,0x78(sp)
80025ebc:  addiu a0,sp,0x28
80025ec0:  move a2,s8
80025ec4:  addiu a3,s2,0x18
80025ec8:  addiu v0,s5,0xc
80025ecc:  sw s0,0x10(sp)
80025ed0:  sw v0,0x14(sp)
80025ed4:  jal 0x80025b20
80025ed8:  _sw v1,0x18(sp)
80025edc:  lw v0,0x0(s8)
80025ee0:  lw v1,0x4(s8)
80025ee4:  sw v0,0x20(sp)
80025ee8:  sw v1,0x24(sp)
80025eec:  lhu v0,0xc(s5)
80025ef0:  nop
80025ef4:  sh v0,0xc(s2)
80025ef8:  lw v0,0x4(s5)
80025efc:  nop
80025f00:  sw v0,0x4(s2)
80025f04:  lw t2,0x7c(sp)
80025f08:  nop
80025f0c:  lw v0,0x0(t2)
80025f10:  lw v1,0x4(t2)
80025f14:  sw v0,0x30(sp)
80025f18:  sw v1,0x34(sp)
80025f1c:  lhu v0,0x24(s5)
80025f20:  nop
80025f24:  sh v0,0x24(s2)
80025f28:  lw v0,0x1c(s5)
80025f2c:  j 0x800260e4
80025f30:  _sw v0,0x1c(s2)
80025f34:  subu v1,s7,a0
80025f38:  sll v1,v1,0xc
80025f3c:  subu v0,s6,a0
80025f40:  div v1,v0
80025f44:  mflo v1
80025f48:  lw a1,0x78(sp)
80025f4c:  lw a2,0x7c(sp)
80025f50:  addiu a0,sp,0x38
80025f54:  addiu a3,s2,0x30
80025f58:  addiu v0,s5,0x18
80025f5c:  addiu s0,s5,0x24
80025f60:  sw v0,0x10(sp)
80025f64:  sw s0,0x14(sp)
80025f68:  jal 0x80025b20
80025f6c:  _sw v1,0x18(sp)
80025f70:  lw t2,0x40(sp)
80025f74:  subu v1,s7,s6
80025f78:  sll v1,v1,0xc
80025f7c:  subu v0,t2,s6
80025f80:  div v1,v0
80025f84:  mflo v1
80025f88:  lw a1,0x7c(sp)
80025f8c:  addiu a0,sp,0x30
80025f90:  move a2,s8
80025f94:  addiu a3,s2,0x24
80025f98:  sw s0,0x10(sp)
80025f9c:  addiu v0,s5,0xc
80025fa0:  sw v0,0x14(sp)
80025fa4:  jal 0x80025b20
80025fa8:  _sw v1,0x18(sp)
80025fac:  lw v0,0x0(s8)
80025fb0:  lw v1,0x4(s8)
80025fb4:  sw v0,0x20(sp)
80025fb8:  sw v1,0x24(sp)
80025fbc:  lhu v0,0xc(s5)
80025fc0:  nop
80025fc4:  sh v0,0xc(s2)
80025fc8:  lw v0,0x4(s5)
80025fcc:  nop
80025fd0:  sw v0,0x4(s2)
80025fd4:  lw t2,0x78(sp)
80025fd8:  nop
80025fdc:  lw v0,0x0(t2)
80025fe0:  lw v1,0x4(t2)
80025fe4:  sw v0,0x28(sp)
80025fe8:  sw v1,0x2c(sp)
80025fec:  lhu v0,0x18(s5)
80025ff0:  nop
80025ff4:  sh v0,0x18(s2)
80025ff8:  lw v0,0x10(s5)
80025ffc:  j 0x800260e4
80026000:  _sw v0,0x10(s2)
80026004:  gte_ldVXY0 0x20(sp)
80026008:  gte_ldVZ0 0x24(sp)
8002600c:  gte_ldVXY1 0x28(sp)
80026010:  gte_ldVZ1 0x2c(sp)
80026014:  gte_ldVXY2 0x30(sp)
80026018:  gte_ldVZ2 0x34(sp)
8002601c:  nRTPT
80026028:  gte_stFLAG v0
8002602c:  nop
80026030:  lui v1,0x7f85
80026034:  ori v1,v1,0xe000
80026038:  and v0,v0,v1
8002603c:  bne v0,zero,0x8002620c
80026040:  _nop
80026044:  NCLIP
80026048:  gte_stMAC0 v0
8002604c:  lw t2,0x80(sp)
80026050:  nop
80026054:  xor v0,v0,t2
80026058:  blez v0,0x8002620c
8002605c:  _nop
80026060:  gte_stSZ0 v1
80026064:  gte_stSZ1 a0
80026068:  nop
8002606c:  slt v0,a0,v1
80026070:  beq v0,zero,0x8002607c
80026074:  _nop
80026078:  move a0,v1
8002607c:  gte_stSZ2 v1
80026080:  nop
80026084:  slt v0,v1,a0
80026088:  beq v0,zero,0x80026094
8002608c:  _nop
80026090:  move v1,a0
80026094:  stsxy3_gt3 s5
800260a0:  lhu v0,0x800(gp)
800260a4:  bgez v1,0x800260b0
800260a8:  _sh v0,0xe(s5)
800260ac:  addiu v1,v1,0x7
800260b0:  lui v0,0x1f80
800260b4:  lw v0,0x0(v0)
800260b8:  sra v1,v1,0x3
800260bc:  sll v1,v1,0x2
800260c0:  addu v1,v1,v0
800260c4:  lw a0,0x0(v1)
800260c8:  sll v0,s5,0x8
800260cc:  srl v0,v0,0x8
800260d0:  sw v0,0x0(v1)
800260d4:  lui v0,0x900
800260d8:  or a0,a0,v0
800260dc:  j 0x8002620c
800260e0:  _sw a0,0x0(s5)
800260e4:  gte_ldVXY0 0x20(sp)
800260e8:  gte_ldVZ0 0x24(sp)
800260ec:  gte_ldVXY1 0x28(sp)
800260f0:  gte_ldVZ1 0x2c(sp)
800260f4:  gte_ldVXY2 0x30(sp)
800260f8:  gte_ldVZ2 0x34(sp)
800260fc:  nRTPT
80026108:  li v0,0x3c
8002610c:  sb v0,0x7(s2)
80026110:  lhu v0,0x1a(s5)
80026114:  nop
80026118:  sh v0,0x1a(s2)
8002611c:  gte_stFLAG v0
80026120:  nop
80026124:  lui v1,0x7f85
80026128:  ori v1,v1,0xe000
8002612c:  and v0,v0,v1
80026130:  bne v0,zero,0x8002620c
80026134:  _nop
80026138:  NCLIP
8002613c:  gte_stMAC0 v0
80026140:  lw t2,0x80(sp)
80026144:  nop
80026148:  xor v0,v0,t2
8002614c:  blez v0,0x8002620c
80026150:  _nop
80026154:  gte_stSZ0 v1
80026158:  gte_stSZ1 a0
8002615c:  nop
80026160:  slt v0,a0,v1
80026164:  beq v0,zero,0x80026170
80026168:  _nop
8002616c:  move a0,v1
80026170:  gte_stSZ2 v1
80026174:  nop
80026178:  slt v0,v1,a0
8002617c:  beq v0,zero,0x80026188
80026180:  _nop
80026184:  move v1,a0
80026188:  stsxy3_gt3 s2
80026194:  gte_ldVXY0 0x38(sp)
80026198:  gte_ldVZ0 0x3c(sp)
8002619c:  nRTPS
800261a8:  lhu v0,0x800(gp)
800261ac:  nop
800261b0:  sh v0,0xe(s2)
800261b4:  gte_stSXY2 0x2c(s2)
800261b8:  bgez v1,0x800261c4
800261bc:  _nop
800261c0:  addiu v1,v1,0x7
800261c4:  lui v0,0x1f80
800261c8:  lw v0,0x0(v0)
800261cc:  sra v1,v1,0x3
800261d0:  sll v1,v1,0x2
800261d4:  addu v1,v1,v0
800261d8:  lw a0,0x0(v1)
800261dc:  sll v0,s2,0x8
800261e0:  srl v0,v0,0x8
800261e4:  sw v0,0x0(v1)
800261e8:  lui v0,0xc00
800261ec:  or a0,a0,v0
800261f0:  sw a0,0x0(s2)
800261f4:  lui v0,0x1f80
800261f8:  lw v0,0x1c(v0)
800261fc:  nop
80026200:  addiu v0,v0,0x34
80026204:  lui at,0x1f80
80026208:  sw v0,0x1c(at)
8002620c:  lw ra,0x6c(sp)
80026210:  lw s8,0x68(sp)
80026214:  lw s7,0x64(sp)
80026218:  lw s6,0x60(sp)
8002621c:  lw s5,0x5c(sp)
80026220:  lw s4,0x58(sp)
80026224:  lw s3,0x54(sp)
80026228:  lw s2,0x50(sp)
8002622c:  lw s1,0x4c(sp)
80026230:  lw s0,0x48(sp)
80026234:  jr ra
80026238:  _addiu sp,sp,0x70
