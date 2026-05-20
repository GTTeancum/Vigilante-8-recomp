# addr: 0x8004e02c  name: RCpolyFT3A
8004e02c:  lw t0,0x48(a3)
8004e030:  lw t1,0x4c(a3)
8004e034:  lw t2,0x50(a3)
8004e038:  gte_stH t9
8004e03c:  lw t4,0x14(t0)
8004e040:  lw t5,0x14(t1)
8004e044:  lw t6,0x14(t2)
8004e048:  sra t8,t9,0x1
8004e04c:  sltu at,t4,t8
8004e050:  beq at,zero,0x8004e074
8004e054:  _sltu at,t5,t8
8004e058:  beq at,zero,0x8004e074
8004e05c:  _sltu at,t6,t8
8004e060:  beq at,zero,0x8004e074
8004e064:  _nop
8004e068:  move v0,a0
8004e06c:  jr ra
8004e070:  _nop
8004e074:  gte_stOFX t9
8004e078:  lw v0,0x4(a1)
8004e07c:  lw v1,0x8(a1)
8004e080:  sra t9,t9,0x10
8004e084:  srl v0,v0,0x1
8004e088:  srl v1,v1,0x1
8004e08c:  addu t8,t9,v0
8004e090:  lh t4,0x10(t0)
8004e094:  lh t5,0x10(t1)
8004e098:  lh t6,0x10(t2)
8004e09c:  slt at,t8,t4
8004e0a0:  beq at,zero,0x8004e0c4
8004e0a4:  _slt at,t8,t5
8004e0a8:  beq at,zero,0x8004e0c4
8004e0ac:  _slt at,t8,t6
8004e0b0:  beq at,zero,0x8004e0c4
8004e0b4:  _nop
8004e0b8:  move v0,a0
8004e0bc:  jr ra
8004e0c0:  _nop
8004e0c4:  subu t8,t9,v0
8004e0c8:  slt at,t4,t8
8004e0cc:  beq at,zero,0x8004e0f0
8004e0d0:  _slt at,t5,t8
8004e0d4:  beq at,zero,0x8004e0f0
8004e0d8:  _slt at,t6,t8
8004e0dc:  beq at,zero,0x8004e0f0
8004e0e0:  _nop
8004e0e4:  move v0,a0
8004e0e8:  jr ra
8004e0ec:  _nop
8004e0f0:  gte_stOFY t9
8004e0f4:  lh t4,0x12(t0)
8004e0f8:  lh t5,0x12(t1)
8004e0fc:  lh t6,0x12(t2)
8004e100:  sra t9,t9,0x10
8004e104:  addu t8,t9,v1
8004e108:  slt at,t8,t4
8004e10c:  beq at,zero,0x8004e130
8004e110:  _slt at,t8,t5
8004e114:  beq at,zero,0x8004e130
8004e118:  _slt at,t8,t6
8004e11c:  beq at,zero,0x8004e130
8004e120:  _nop
8004e124:  move v0,a0
8004e128:  jr ra
8004e12c:  _nop
8004e130:  subu t8,t9,v1
8004e134:  slt at,t4,t8
8004e138:  beq at,zero,0x8004e15c
8004e13c:  _slt at,t5,t8
8004e140:  beq at,zero,0x8004e15c
8004e144:  _slt at,t6,t8
8004e148:  beq at,zero,0x8004e15c
8004e14c:  _nop
8004e150:  move v0,a0
8004e154:  jr ra
8004e158:  _nop
8004e15c:  lh t4,0x0(t0)
8004e160:  lh t5,0x0(t1)
8004e164:  lh t6,0x0(t2)
8004e168:  add t7,t4,t5
8004e16c:  add t8,t5,t6
8004e170:  add t9,t6,t4
8004e174:  sra t7,t7,0x1
8004e178:  sra t8,t8,0x1
8004e17c:  sra t9,t9,0x1
8004e180:  sh t7,0x0(a3)
8004e184:  sh t8,0x18(a3)
8004e188:  sh t9,0x30(a3)
8004e18c:  lh t4,0x2(t0)
8004e190:  lh t5,0x2(t1)
8004e194:  lh t6,0x2(t2)
8004e198:  add t7,t4,t5
8004e19c:  add t8,t5,t6
8004e1a0:  add t9,t6,t4
8004e1a4:  sra t7,t7,0x1
8004e1a8:  sra t8,t8,0x1
8004e1ac:  sra t9,t9,0x1
8004e1b0:  sh t7,0x2(a3)
8004e1b4:  sh t8,0x1a(a3)
8004e1b8:  sh t9,0x32(a3)
8004e1bc:  lh t4,0x4(t0)
8004e1c0:  lh t5,0x4(t1)
8004e1c4:  lh t6,0x4(t2)
8004e1c8:  add t7,t4,t5
8004e1cc:  add t8,t5,t6
8004e1d0:  add t9,t6,t4
8004e1d4:  sra t7,t7,0x1
8004e1d8:  sra t8,t8,0x1
8004e1dc:  sra t9,t9,0x1
8004e1e0:  sh t7,0x4(a3)
8004e1e4:  sh t8,0x1c(a3)
8004e1e8:  sh t9,0x34(a3)
8004e1ec:  ldv0 a3
8004e1f4:  gte_ldVXY1 0x18(a3)
8004e1f8:  gte_ldVZ1 0x1c(a3)
8004e1fc:  gte_ldVXY2 0x30(a3)
8004e200:  gte_ldVZ2 0x34(a3)
8004e204:  lbu t4,0x8(t0)
8004e208:  RTPT
8004e20c:  lbu t5,0x8(t1)
8004e210:  lbu t6,0x8(t2)
8004e214:  addu t7,t4,t5
8004e218:  addu t8,t5,t6
8004e21c:  addu t9,t6,t4
8004e220:  srl t7,t7,0x1
8004e224:  srl t8,t8,0x1
8004e228:  srl t9,t9,0x1
8004e22c:  sb t7,0x8(a3)
8004e230:  sb t8,0x20(a3)
8004e234:  sb t9,0x38(a3)
8004e238:  lbu t4,0x9(t0)
8004e23c:  lbu t5,0x9(t1)
8004e240:  lbu t6,0x9(t2)
8004e244:  addu t7,t4,t5
8004e248:  addu t8,t5,t6
8004e24c:  addu t9,t6,t4
8004e250:  srl t7,t7,0x1
8004e254:  srl t8,t8,0x1
8004e258:  srl t9,t9,0x1
8004e25c:  sb t7,0x9(a3)
8004e260:  sb t8,0x21(a3)
8004e264:  sb t9,0x39(a3)
8004e268:  lw t4,0x0(a1)
8004e26c:  addiu a2,a2,0x1
8004e270:  bne t4,a2,0x8004e2e8
8004e274:  _nop
8004e278:  gte_stSXY0 0x10(a3)
8004e27c:  gte_stSXY1 0x28(a3)
8004e280:  gte_stSXY2 0x40(a3)
8004e284:  move v1,ra
8004e288:  lw t0,0x4c(a3)
8004e28c:  addiu t1,a3,0x18
8004e290:  addiu t2,a3,0x0
8004e294:  jal 0x8004e3a0
8004e298:  _nop
8004e29c:  addiu t0,a3,0x0
8004e2a0:  addiu t1,a3,0x18
8004e2a4:  addiu t2,a3,0x30
8004e2a8:  jal 0x8004e3a0
8004e2ac:  _nop
8004e2b0:  lw t0,0x48(a3)
8004e2b4:  addiu t1,a3,0x0
8004e2b8:  addiu t2,a3,0x30
8004e2bc:  jal 0x8004e3a0
8004e2c0:  _nop
8004e2c4:  lw t0,0x50(a3)
8004e2c8:  addiu t1,a3,0x30
8004e2cc:  addiu t2,a3,0x18
8004e2d0:  jal 0x8004e3a0
8004e2d4:  _nop
8004e2d8:  move ra,v1
8004e2dc:  addiu a2,a2,-0x1
8004e2e0:  b 0x8004e394
8004e2e4:  _nop
8004e2e8:  gte_stSZ1 0x14(a3)
8004e2ec:  gte_stSZ2 0x2c(a3)
8004e2f0:  gte_stSZ3 0x44(a3)
8004e2f4:  gte_stSXY0 0x10(a3)
8004e2f8:  gte_stSXY1 0x28(a3)
8004e2fc:  gte_stSXY2 0x40(a3)
8004e300:  addiu a3,a3,0x58
8004e304:  sw ra,0x54(a3)
8004e308:  lw t4,-0x10(a3)
8004e30c:  addiu t5,a3,-0x58
8004e310:  addiu t6,a3,-0x28
8004e314:  sw t4,0x48(a3)
8004e318:  sw t5,0x4c(a3)
8004e31c:  sw t6,0x50(a3)
8004e320:  jal 0x8004e02c
8004e324:  _nop
8004e328:  lw t4,-0xc(a3)
8004e32c:  addiu t5,a3,-0x40
8004e330:  addiu t6,a3,-0x58
8004e334:  sw t4,0x48(a3)
8004e338:  sw t5,0x4c(a3)
8004e33c:  sw t6,0x50(a3)
8004e340:  jal 0x8004e02c
8004e344:  _nop
8004e348:  lw t4,-0x8(a3)
8004e34c:  addiu t5,a3,-0x28
8004e350:  addiu t6,a3,-0x40
8004e354:  sw t4,0x48(a3)
8004e358:  sw t5,0x4c(a3)
8004e35c:  sw t6,0x50(a3)
8004e360:  jal 0x8004e02c
8004e364:  _nop
8004e368:  addiu t4,a3,-0x58
8004e36c:  addiu t5,a3,-0x40
8004e370:  addiu t6,a3,-0x28
8004e374:  sw t4,0x48(a3)
8004e378:  sw t5,0x4c(a3)
8004e37c:  sw t6,0x50(a3)
8004e380:  jal 0x8004e02c
8004e384:  _nop
8004e388:  lw ra,0x54(a3)
8004e38c:  addiu a3,a3,-0x58
8004e390:  addiu a2,a2,-0x1
8004e394:  move v0,a0
8004e398:  jr ra
8004e39c:  _nop
