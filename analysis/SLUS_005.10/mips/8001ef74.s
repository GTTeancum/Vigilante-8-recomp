# addr: 0x8001ef74  name: FUN_8001ef74
8001ef74:  addiu sp,sp,-0x68
8001ef78:  sw s0,0x40(sp)
8001ef7c:  move s0,a0
8001ef80:  sw s5,0x54(sp)
8001ef84:  move s5,a1
8001ef88:  sw s8,0x60(sp)
8001ef8c:  move s8,a2
8001ef90:  sw s7,0x5c(sp)
8001ef94:  sw ra,0x64(sp)
8001ef98:  sw s6,0x58(sp)
8001ef9c:  sw s4,0x50(sp)
8001efa0:  sw s3,0x4c(sp)
8001efa4:  sw s2,0x48(sp)
8001efa8:  sw s1,0x44(sp)
8001efac:  lw s6,0x78(sp)
8001efb0:  beq s0,zero,0x8001f378
8001efb4:  _move s7,a3
8001efb8:  lw v0,0x0(s7)
8001efbc:  lw v1,0x14(s5)
8001efc0:  nop
8001efc4:  subu v0,v0,v1
8001efc8:  sw v0,0x10(sp)
8001efcc:  lw v0,0x4(s7)
8001efd0:  lw v1,0x18(s5)
8001efd4:  addiu a0,sp,0x10
8001efd8:  subu v0,v0,v1
8001efdc:  sw v0,0x4(a0)
8001efe0:  lw v0,0x8(s7)
8001efe4:  lw v1,0x1c(s5)
8001efe8:  nop
8001efec:  subu v0,v0,v1
8001eff0:  sw v0,0x8(a0)
8001eff4:  lhu v0,0x0(s0)
8001eff8:  nop
8001effc:  beq v0,zero,0x8001f378
8001f000:  _li v0,0x1
8001f004:  lhu v1,0x0(s0)
8001f008:  nop
8001f00c:  beq v1,v0,0x8001f024
8001f010:  _li v0,0x2
8001f014:  beq v1,v0,0x8001f180
8001f018:  _move a0,s5
8001f01c:  j 0x8001eff4
8001f020:  _nop
8001f024:  move a0,s5
8001f028:  addiu a1,sp,0x10
8001f02c:  addiu a2,sp,0x20
8001f030:  jal 0x8004352c
8001f034:  _addiu s1,s0,0x4
8001f038:  lw v1,0x20(sp)
8001f03c:  lw v0,0xc(s1)
8001f040:  nop
8001f044:  slt v0,v1,v0
8001f048:  beq v0,zero,0x8001f178
8001f04c:  _nop
8001f050:  lw v0,0x4(s0)
8001f054:  nop
8001f058:  slt v0,v0,v1
8001f05c:  beq v0,zero,0x8001f178
8001f060:  _nop
8001f064:  lw v1,0x28(sp)
8001f068:  lw v0,0x14(s1)
8001f06c:  nop
8001f070:  slt v0,v1,v0
8001f074:  beq v0,zero,0x8001f178
8001f078:  _nop
8001f07c:  lw v0,0x8(s1)
8001f080:  nop
8001f084:  slt v0,v0,v1
8001f088:  beq v0,zero,0x8001f178
8001f08c:  _nop
8001f090:  lw a0,0x24(sp)
8001f094:  lw v0,0x10(s1)
8001f098:  nop
8001f09c:  slt v0,a0,v0
8001f0a0:  beq v0,zero,0x8001f178
8001f0a4:  _nop
8001f0a8:  lw v1,0x4(s1)
8001f0ac:  nop
8001f0b0:  addiu v0,v1,0x2800
8001f0b4:  slt v0,a0,v0
8001f0b8:  beq v0,zero,0x8001f178
8001f0bc:  _addiu v0,v1,-0x2800
8001f0c0:  slt v0,v0,a0
8001f0c4:  beq v0,zero,0x8001f178
8001f0c8:  _nop
8001f0cc:  lw v0,0x18(s5)
8001f0d0:  nop
8001f0d4:  addu v0,v0,v1
8001f0d8:  slt v0,v0,s8
8001f0dc:  bne v0,zero,0x8001f118
8001f0e0:  _lui v0,0x1
8001f0e4:  lw v1,0x4(s7)
8001f0e8:  addu v0,s8,v0
8001f0ec:  slt v0,v0,v1
8001f0f0:  bne v0,zero,0x8001f118
8001f0f4:  _nop
8001f0f8:  lw a0,0x0(s7)
8001f0fc:  lw a1,0x8(s7)
8001f100:  jal 0x800255f4
8001f104:  _nop
8001f108:  lhu v0,0x2(v0)
8001f10c:  nop
8001f110:  bne v0,zero,0x8001eff4
8001f114:  _addiu s0,s0,0x1c
8001f118:  beq s6,zero,0x8001f150
8001f11c:  _nop
8001f120:  lhu v0,0x2(s5)
8001f124:  nop
8001f128:  subu v0,zero,v0
8001f12c:  sh v0,0x0(s6)
8001f130:  lhu v0,0x8(s5)
8001f134:  nop
8001f138:  subu v0,zero,v0
8001f13c:  sh v0,0x2(s6)
8001f140:  lhu v0,0xe(s5)
8001f144:  nop
8001f148:  subu v0,zero,v0
8001f14c:  sh v0,0x4(s6)
8001f150:  lw v0,0x4(s1)
8001f154:  move a0,s5
8001f158:  addiu a1,sp,0x20
8001f15c:  move a2,a1
8001f160:  jal 0x80043358
8001f164:  _sw v0,0x24(sp)
8001f168:  lw v1,0x18(s5)
8001f16c:  lw v0,0x24(sp)
8001f170:  j 0x8001f37c
8001f174:  _addu v0,v1,v0
8001f178:  j 0x8001eff4
8001f17c:  _addiu s0,s0,0x1c
8001f180:  jal 0x8004d344
8001f184:  _move s3,s0
8001f188:  lhu v0,0x2(s3)
8001f18c:  lui s4,0x7fff
8001f190:  lui s2,0x8001
8001f194:  beq v0,zero,0x8001f2e4
8001f198:  _clear s0
8001f19c:  li s1,0x4
8001f1a0:  addu v0,s3,s1
8001f1a4:  ldv0 v0
8001f1ac:  nRTV0
8001f1b8:  lw v1,0x8(v0)
8001f1bc:  li v0,0x1000
8001f1c0:  mult v1,v0
8001f1c4:  mfhi a1
8001f1c8:  mflo a0
8001f1cc:  gte_stIR1 v1
8001f1d0:  lw v0,0x10(sp)
8001f1d4:  nop
8001f1d8:  mult v1,v0
8001f1dc:  mfhi t1
8001f1e0:  mflo t0
8001f1e4:  sw t0,0x38(sp)
8001f1e8:  sw t1,0x3c(sp)
8001f1ec:  sltu v0,a0,t0
8001f1f0:  subu a0,a0,t0
8001f1f4:  subu a1,a1,t1
8001f1f8:  subu a1,a1,v0
8001f1fc:  gte_stIR3 v1
8001f200:  lw v0,0x18(sp)
8001f204:  nop
8001f208:  mult v1,v0
8001f20c:  mfhi t1
8001f210:  mflo t0
8001f214:  sw t0,0x38(sp)
8001f218:  sw t1,0x3c(sp)
8001f21c:  sltu v0,a0,t0
8001f220:  subu a0,a0,t0
8001f224:  subu a1,a1,t1
8001f228:  subu a1,a1,v0
8001f22c:  gte_stIR2 v0
8001f230:  nop
8001f234:  bgez v0,0x8001f280
8001f238:  _nop
8001f23c:  gte_stIR2 a2
8001f240:  jal 0x8004779c
8001f244:  _sra a3,a2,0x1f
8001f248:  move a0,v0
8001f24c:  move a1,v1
8001f250:  slt v0,s2,a0
8001f254:  beq v0,zero,0x8001f2d0
8001f258:  _nop
8001f25c:  move s2,a0
8001f260:  gte_stIR1 t4
8001f264:  gte_stIR2 t5
8001f268:  gte_stIR3 t6
8001f26c:  sh t4,0x30(sp)
8001f270:  sh t5,0x32(sp)
8001f274:  sh t6,0x34(sp)
8001f278:  j 0x8001f2d0
8001f27c:  _nop
8001f280:  gte_stIR2 v0
8001f284:  nop
8001f288:  blez v0,0x8001f2c8
8001f28c:  _nop
8001f290:  gte_stIR2 a2
8001f294:  jal 0x8004779c
8001f298:  _sra a3,a2,0x1f
8001f29c:  lw a0,0x14(sp)
8001f2a0:  move a2,v0
8001f2a4:  move a3,v1
8001f2a8:  move v0,a2
8001f2ac:  slt a0,v0,a0
8001f2b0:  bne a0,zero,0x8001f358
8001f2b4:  _slt v0,v0,s4
8001f2b8:  beq v0,zero,0x8001f2d0
8001f2bc:  _nop
8001f2c0:  j 0x8001f2d0
8001f2c4:  _move s4,a2
8001f2c8:  bltz a1,0x8001f358
8001f2cc:  _nop
8001f2d0:  lhu v0,0x2(s3)
8001f2d4:  addiu s0,s0,0x1
8001f2d8:  slt v0,s0,v0
8001f2dc:  bne v0,zero,0x8001f1a0
8001f2e0:  _addiu s1,s1,0xc
8001f2e4:  slt v0,s2,s4
8001f2e8:  beq v0,zero,0x8001f358
8001f2ec:  _nop
8001f2f0:  lw v0,0x18(s5)
8001f2f4:  nop
8001f2f8:  addu v1,s2,v0
8001f2fc:  slt v0,v1,s8
8001f300:  beq v0,zero,0x8001f358
8001f304:  _nop
8001f308:  lw v0,0x4(s7)
8001f30c:  nop
8001f310:  addiu v0,v0,-0x2800
8001f314:  slt v0,v0,v1
8001f318:  beq v0,zero,0x8001f358
8001f31c:  _nop
8001f320:  lh v0,0x32(sp)
8001f324:  nop
8001f328:  slti v0,v0,-0x800
8001f32c:  beq v0,zero,0x8001f358
8001f330:  _nop
8001f334:  beq s6,zero,0x8001f34c
8001f338:  _nop
8001f33c:  lw t0,0x30(sp)
8001f340:  lw t1,0x34(sp)
8001f344:  sw t0,0x0(s6)
8001f348:  sw t1,0x4(s6)
8001f34c:  lw v0,0x18(s5)
8001f350:  j 0x8001f37c
8001f354:  _addu v0,v0,s2
8001f358:  lhu v1,0x2(s3)
8001f35c:  nop
8001f360:  sll v0,v1,0x1
8001f364:  addu v0,v0,v1
8001f368:  sll v0,v0,0x2
8001f36c:  addiu v0,v0,0x4
8001f370:  j 0x8001eff4
8001f374:  _addu s0,s3,v0
8001f378:  clear v0
8001f37c:  lw ra,0x64(sp)
8001f380:  lw s8,0x60(sp)
8001f384:  lw s7,0x5c(sp)
8001f388:  lw s6,0x58(sp)
8001f38c:  lw s5,0x54(sp)
8001f390:  lw s4,0x50(sp)
8001f394:  lw s3,0x4c(sp)
8001f398:  lw s2,0x48(sp)
8001f39c:  lw s1,0x44(sp)
8001f3a0:  lw s0,0x40(sp)
8001f3a4:  jr ra
8001f3a8:  _addiu sp,sp,0x68
