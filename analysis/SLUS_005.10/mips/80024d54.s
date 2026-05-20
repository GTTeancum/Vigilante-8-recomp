# addr: 0x80024d54  name: FUN_80024d54
80024d54:  addiu sp,sp,-0x38
80024d58:  sw s2,0x18(sp)
80024d5c:  move s2,a0
80024d60:  sw s7,0x2c(sp)
80024d64:  move s7,a1
80024d68:  sw s5,0x24(sp)
80024d6c:  clear s5
80024d70:  sw ra,0x34(sp)
80024d74:  sw s8,0x30(sp)
80024d78:  sw s6,0x28(sp)
80024d7c:  sw s4,0x20(sp)
80024d80:  sw s3,0x1c(sp)
80024d84:  sw s1,0x14(sp)
80024d88:  sw s0,0x10(sp)
80024d8c:  sw a2,0x40(sp)
80024d90:  jal 0x80015010
80024d94:  _sw a3,0x44(sp)
80024d98:  move s8,v0
80024d9c:  lui v0,0x8007
80024da0:  addiu s1,v0,0x38a0
80024da4:  li s0,0x3fe
80024da8:  sw s1,0x7ec(gp)
80024dac:  addiu v0,s1,0x1c
80024db0:  sw v0,0x0(s1)
80024db4:  addiu s0,s0,-0x1
80024db8:  bgez s0,0x80024dac
80024dbc:  _move s1,v0
80024dc0:  sw zero,0x0(s1)
80024dc4:  lh a0,0x2(s7)
80024dc8:  lh a1,0xa(s7)
80024dcc:  jal 0x800247dc
80024dd0:  _nop
80024dd4:  move s4,v0
80024dd8:  lbu v0,0x10(s4)
80024ddc:  lw v1,0x8(s4)
80024de0:  sll v0,v0,0x1
80024de4:  addu v1,v1,v0
80024de8:  lhu v0,0x2(v1)
80024dec:  nop
80024df0:  bne v0,zero,0x80024e1c
80024df4:  _nop
80024df8:  jal 0x80024998
80024dfc:  _move a0,s4
80024e00:  move s3,v0
80024e04:  beq s3,zero,0x800251cc
80024e08:  _clear v0
80024e0c:  lw v0,0x7ec(gp)
80024e10:  sw s4,0x7ec(gp)
80024e14:  sw v0,0x0(s4)
80024e18:  move s4,s3
80024e1c:  lh a0,0x2(s2)
80024e20:  lh a1,0xa(s2)
80024e24:  jal 0x800247dc
80024e28:  _nop
80024e2c:  move s2,v0
80024e30:  lbu v0,0x10(s2)
80024e34:  lw v1,0x8(s2)
80024e38:  sll v0,v0,0x1
80024e3c:  addu v1,v1,v0
80024e40:  lhu v0,0x2(v1)
80024e44:  nop
80024e48:  bne v0,zero,0x80024e74
80024e4c:  _nop
80024e50:  jal 0x80024998
80024e54:  _move a0,s2
80024e58:  move s3,v0
80024e5c:  beq s3,zero,0x800251cc
80024e60:  _clear v0
80024e64:  lw v0,0x7ec(gp)
80024e68:  sw s2,0x7ec(gp)
80024e6c:  sw v0,0x0(s2)
80024e70:  move s2,s3
80024e74:  sw zero,0x18(s2)
80024e78:  lh a1,0xc(s4)
80024e7c:  lh a2,0xe(s4)
80024e80:  jal 0x80024cc0
80024e84:  _move a0,s1
80024e88:  sw v0,0x14(s2)
80024e8c:  sw zero,0x0(s2)
80024e90:  beq s2,zero,0x800251b8
80024e94:  _sw zero,0x4(s2)
80024e98:  li s6,0x1
80024e9c:  jal 0x80015010
80024ea0:  _nop
80024ea4:  lw t1,0x40(sp)
80024ea8:  subu v0,v0,s8
80024eac:  sltu v0,t1,v0
80024eb0:  beq v0,zero,0x80024ec8
80024eb4:  _move s1,s2
80024eb8:  lw t1,0x44(sp)
80024ebc:  nop
80024ec0:  bne t1,zero,0x800251b8
80024ec4:  _nop
80024ec8:  lw s2,0x0(s2)
80024ecc:  bne v0,zero,0x80024efc
80024ed0:  _nop
80024ed4:  lw v1,0x8(s1)
80024ed8:  lw v0,0x8(s4)
80024edc:  nop
80024ee0:  bne v1,v0,0x80024ffc
80024ee4:  _nop
80024ee8:  lbu v1,0x10(s1)
80024eec:  lbu v0,0x10(s4)
80024ef0:  nop
80024ef4:  bne v1,v0,0x80024ffc
80024ef8:  _nop
80024efc:  lbu v0,0x10(s1)
80024f00:  lw v1,0x8(s1)
80024f04:  sll v0,v0,0x1
80024f08:  addu v1,v1,v0
80024f0c:  lhu v0,0x2(v1)
80024f10:  nop
80024f14:  andi v0,v0,0x9fff
80024f18:  sh v0,0x2(v1)
80024f1c:  lw v0,0x4(s1)
80024f20:  nop
80024f24:  beq v0,zero,0x80024f3c
80024f28:  _clear s0
80024f2c:  lw v0,0x4(v0)
80024f30:  nop
80024f34:  bne v0,zero,0x80024f2c
80024f38:  _addiu s0,s0,0x1
80024f3c:  sltu v0,zero,s0
80024f40:  subu s0,s0,v0
80024f44:  addiu a0,s0,0x2
80024f48:  jal 0x800116f4
80024f4c:  _sll a0,a0,0x2
80024f50:  move s3,v0
80024f54:  sll v1,s0,0x2
80024f58:  addu v1,v1,s3
80024f5c:  sh zero,0x6(v1)
80024f60:  sh zero,0x4(v1)
80024f64:  lh v0,0x2(s7)
80024f68:  nop
80024f6c:  sh v0,0x0(v1)
80024f70:  lh v0,0xa(s7)
80024f74:  addiu s0,s0,-0x1
80024f78:  sh v0,0x2(v1)
80024f7c:  li v0,-0x1
80024f80:  beq s0,v0,0x80024fe4
80024f84:  _move a2,v0
80024f88:  sll v0,s0,0x2
80024f8c:  addu a1,v0,s3
80024f90:  lw s1,0x4(s1)
80024f94:  nop
80024f98:  lbu v0,0x11(s1)
80024f9c:  lhu a0,0xc(s1)
80024fa0:  sllv v0,s6,v0
80024fa4:  srl v1,v0,0x1f
80024fa8:  addu v0,v0,v1
80024fac:  sra v0,v0,0x1
80024fb0:  addu a0,a0,v0
80024fb4:  sh a0,0x0(a1)
80024fb8:  lbu v0,0x11(s1)
80024fbc:  lhu a0,0xe(s1)
80024fc0:  addiu s0,s0,-0x1
80024fc4:  sllv v0,s6,v0
80024fc8:  srl v1,v0,0x1f
80024fcc:  addu v0,v0,v1
80024fd0:  sra v0,v0,0x1
80024fd4:  addu a0,a0,v0
80024fd8:  sh a0,0x2(a1)
80024fdc:  bne s0,a2,0x80024f90
80024fe0:  _addiu a1,a1,-0x4
80024fe4:  jal 0x8002479c
80024fe8:  _move a0,s2
80024fec:  jal 0x8002479c
80024ff0:  _move a0,s5
80024ff4:  j 0x800251cc
80024ff8:  _move v0,s3
80024ffc:  jal 0x80024998
80025000:  _move a0,s1
80025004:  move s3,v0
80025008:  beq s3,zero,0x8002518c
8002500c:  _move s0,s3
80025010:  lw v1,0x8(s0)
80025014:  lbu v0,0x10(s0)
80025018:  lw s3,0x0(s3)
8002501c:  lbu a0,0x11(s0)
80025020:  lw a2,0x8(s0)
80025024:  sll v0,v0,0x1
80025028:  addu v1,v1,v0
8002502c:  lbu v1,0x2(v1)
80025030:  lw v0,0x18(s1)
80025034:  lbu a3,0x10(s0)
80025038:  sllv v1,v1,a0
8002503c:  addu v0,v0,v1
80025040:  sw v0,0x18(s0)
80025044:  sll v0,a3,0x1
80025048:  addu v1,a2,v0
8002504c:  lhu a0,0x2(v1)
80025050:  nop
80025054:  andi v0,a0,0x4000
80025058:  beq v0,zero,0x80025148
8002505c:  _clear t0
80025060:  andi v0,a0,0x2000
80025064:  beq v0,zero,0x80025070
80025068:  _move a1,s2
8002506c:  move a1,s5
80025070:  lw v0,0x8(a1)
80025074:  nop
80025078:  bne a2,v0,0x80025090
8002507c:  _nop
80025080:  lbu v0,0x10(a1)
80025084:  nop
80025088:  beq a3,v0,0x800250c0
8002508c:  _nop
80025090:  move t0,a1
80025094:  lw a1,0x0(a1)
80025098:  lw v1,0x8(s0)
8002509c:  lw v0,0x8(a1)
800250a0:  nop
800250a4:  bne v1,v0,0x80025090
800250a8:  _nop
800250ac:  lbu v1,0x10(s0)
800250b0:  lbu v0,0x10(a1)
800250b4:  nop
800250b8:  bne v1,v0,0x80025090
800250bc:  _nop
800250c0:  lw v0,0x7ec(gp)
800250c4:  nop
800250c8:  sw v0,0x0(s0)
800250cc:  lw v1,0x18(a1)
800250d0:  lw v0,0x18(s0)
800250d4:  sw s0,0x7ec(gp)
800250d8:  subu a2,v1,v0
800250dc:  blez a2,0x80025184
800250e0:  _nop
800250e4:  beq t0,zero,0x800250f8
800250e8:  _andi v0,a0,0x2000
800250ec:  lw v0,0x0(a1)
800250f0:  j 0x80025110
800250f4:  _sw v0,0x0(t0)
800250f8:  beq v0,zero,0x8002510c
800250fc:  _nop
80025100:  lw s5,0x0(a1)
80025104:  j 0x80025110
80025108:  _nop
8002510c:  lw s2,0x0(a1)
80025110:  lbu v0,0x10(a1)
80025114:  lw v1,0x8(a1)
80025118:  sll v0,v0,0x1
8002511c:  addu v1,v1,v0
80025120:  andi v0,a0,0xdfff
80025124:  sh v0,0x2(v1)
80025128:  lw v1,0x18(s0)
8002512c:  lw v0,0x14(a1)
80025130:  move a0,s2
80025134:  sw s1,0x4(a1)
80025138:  subu v0,v0,a2
8002513c:  sw v1,0x18(a1)
80025140:  j 0x80025178
80025144:  _sw v0,0x14(a1)
80025148:  ori v0,a0,0x4000
8002514c:  sh v0,0x2(v1)
80025150:  lh a1,0xc(s4)
80025154:  lh a2,0xe(s4)
80025158:  jal 0x80024cc0
8002515c:  _move a0,s0
80025160:  lw v1,0x18(s0)
80025164:  move a0,s2
80025168:  move a1,s0
8002516c:  sw s1,0x4(a1)
80025170:  addu v1,v1,v0
80025174:  sw v1,0x14(a1)
80025178:  jal 0x80024748
8002517c:  _nop
80025180:  move s2,v0
80025184:  bne s3,zero,0x80025010
80025188:  _move s0,s3
8002518c:  sw s5,0x0(s1)
80025190:  move s5,s1
80025194:  lbu v0,0x10(s5)
80025198:  lw v1,0x8(s5)
8002519c:  sll v0,v0,0x1
800251a0:  addu v1,v1,v0
800251a4:  lhu v0,0x2(v1)
800251a8:  nop
800251ac:  ori v0,v0,0x6000
800251b0:  bne s2,zero,0x80024e9c
800251b4:  _sh v0,0x2(v1)
800251b8:  jal 0x8002479c
800251bc:  _move a0,s2
800251c0:  jal 0x8002479c
800251c4:  _move a0,s5
800251c8:  clear v0
800251cc:  lw ra,0x34(sp)
800251d0:  lw s8,0x30(sp)
800251d4:  lw s7,0x2c(sp)
800251d8:  lw s6,0x28(sp)
800251dc:  lw s5,0x24(sp)
800251e0:  lw s4,0x20(sp)
800251e4:  lw s3,0x1c(sp)
800251e8:  lw s2,0x18(sp)
800251ec:  lw s1,0x14(sp)
800251f0:  lw s0,0x10(sp)
800251f4:  jr ra
800251f8:  _addiu sp,sp,0x38
