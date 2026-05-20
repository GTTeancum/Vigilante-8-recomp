# addr: 0x8002d054  name: FUN_8002d054
8002d054:  addiu sp,sp,-0x20
8002d058:  sw s0,0x10(sp)
8002d05c:  move s0,a0
8002d060:  sw ra,0x1c(sp)
8002d064:  sw s2,0x18(sp)
8002d068:  sw s1,0x14(sp)
8002d06c:  lb v0,0x5(s0)
8002d070:  lw a0,0x0(s0)
8002d074:  lui v1,0x1f80
8002d078:  ori v1,v1,0x1bf0
8002d07c:  sll v0,v0,0x4
8002d080:  addu s2,v0,v1
8002d084:  lui v0,0x800
8002d088:  and v0,a0,v0
8002d08c:  beq v0,zero,0x8002d0b0
8002d090:  _lui v0,0x10
8002d094:  jal 0x800446dc
8002d098:  _addiu a0,s0,0x24
8002d09c:  lb a0,0x5(s0)
8002d0a0:  jal 0x80044574
8002d0a4:  _move a1,v0
8002d0a8:  j 0x8002d27c
8002d0ac:  _nop
8002d0b0:  and v0,a0,v0
8002d0b4:  beq v0,zero,0x8002d15c
8002d0b8:  _lui v1,0x8006
8002d0bc:  lb a0,0xb2(s0)
8002d0c0:  addiu v1,v1,-0x1398
8002d0c4:  addiu v0,a0,0x1
8002d0c8:  sll v0,v0,0x1
8002d0cc:  addu v0,v0,v1
8002d0d0:  lhu v1,0x0(v0)
8002d0d4:  lw v0,0x8c(s0)
8002d0d8:  nop
8002d0dc:  mult v0,v1
8002d0e0:  lbu v1,0xb2(s0)
8002d0e4:  mflo v0
8002d0e8:  bgez v0,0x8002d0f8
8002d0ec:  _sra s1,v0,0xc
8002d0f0:  addiu v0,v0,0xfff
8002d0f4:  sra s1,v0,0xc
8002d0f8:  slti v0,s1,0x300
8002d0fc:  beq v0,zero,0x8002d114
8002d100:  _slti v0,a0,0x2
8002d104:  bne v0,zero,0x8002d118
8002d108:  _slti v0,s1,0x801
8002d10c:  addiu v0,v1,-0x1
8002d110:  sb v0,0xb2(s0)
8002d114:  slti v0,s1,0x801
8002d118:  bne v0,zero,0x8002d144
8002d11c:  _li v1,0x300
8002d120:  lbu v1,0xb2(s0)
8002d124:  nop
8002d128:  sltiu v0,v1,0x3
8002d12c:  beq v0,zero,0x8002d140
8002d130:  _sll v0,v1,0x18
8002d134:  bltz v0,0x8002d140
8002d138:  _addiu v0,v1,0x1
8002d13c:  sb v0,0xb2(s0)
8002d140:  li v1,0x300
8002d144:  slt v0,v1,s1
8002d148:  beq v0,zero,0x8002d154
8002d14c:  _nop
8002d150:  move v1,s1
8002d154:  j 0x8002d170
8002d158:  _move s1,v1
8002d15c:  lh v0,0xa6(s0)
8002d160:  nop
8002d164:  blez v0,0x8002d170
8002d168:  _li s1,0x300
8002d16c:  li s1,0x800
8002d170:  lh v0,0xd4(s0)
8002d174:  li v1,-0x80
8002d178:  lhu a1,0xd4(s0)
8002d17c:  subu a0,s1,v0
8002d180:  slt v0,v1,a0
8002d184:  beq v0,zero,0x8002d190
8002d188:  _nop
8002d18c:  move v1,a0
8002d190:  li a0,0x80
8002d194:  slt v0,v1,a0
8002d198:  beq v0,zero,0x8002d1a8
8002d19c:  _addu v0,a1,a0
8002d1a0:  move a0,v1
8002d1a4:  addu v0,a1,a0
8002d1a8:  sh v0,0xd4(s0)
8002d1ac:  sh v0,0x4(s2)
8002d1b0:  lh v1,0x6(s0)
8002d1b4:  lui a0,0x8006
8002d1b8:  addiu a0,a0,0x5c28
8002d1bc:  nor v1,zero,v1
8002d1c0:  sll v0,v1,0x1
8002d1c4:  addu v0,v0,v1
8002d1c8:  sll v0,v0,0x3
8002d1cc:  addu v0,v0,a0
8002d1d0:  lw v0,0x8(v0)
8002d1d4:  nop
8002d1d8:  andi v0,v0,0x100
8002d1dc:  beq v0,zero,0x8002d208
8002d1e0:  _li a0,0x1000
8002d1e4:  lh v0,0xd6(s0)
8002d1e8:  nop
8002d1ec:  addiu v1,v0,0x80
8002d1f0:  slt v0,v1,a0
8002d1f4:  beq v0,zero,0x8002d200
8002d1f8:  _nop
8002d1fc:  move a0,v1
8002d200:  j 0x8002d228
8002d204:  _sh a0,0xd6(s0)
8002d208:  lh v0,0xd6(s0)
8002d20c:  li v1,0x800
8002d210:  addiu a0,v0,-0x80
8002d214:  slt v0,v1,a0
8002d218:  beq v0,zero,0x8002d224
8002d21c:  _nop
8002d220:  move v1,a0
8002d224:  sh v1,0xd6(s0)
8002d228:  jal 0x800446dc
8002d22c:  _addiu a0,s0,0x24
8002d230:  lh v1,0xd6(s0)
8002d234:  move s1,v0
8002d238:  andi v0,s1,0xffff
8002d23c:  mult v0,v1
8002d240:  mflo v0
8002d244:  bgez v0,0x8002d250
8002d248:  _nop
8002d24c:  addiu v0,v0,0xfff
8002d250:  sra v0,v0,0xc
8002d254:  sh v0,0x0(s2)
8002d258:  lh v1,0xd6(s0)
8002d25c:  sra v0,s1,0x10
8002d260:  mult v0,v1
8002d264:  mflo v0
8002d268:  bgez v0,0x8002d274
8002d26c:  _nop
8002d270:  addiu v0,v0,0xfff
8002d274:  sra v0,v0,0xc
8002d278:  sh v0,0x2(s2)
8002d27c:  lw v0,0x0(s0)
8002d280:  lui v1,0x40
8002d284:  and v0,v0,v1
8002d288:  beq v0,zero,0x8002d2d0
8002d28c:  _nop
8002d290:  jal 0x8004410c
8002d294:  _nop
8002d298:  lh v1,0x18(s0)
8002d29c:  lw a1,0x5f8(gp)
8002d2a0:  blez v1,0x8002d2ac
8002d2a4:  _li a2,0x1e
8002d2a8:  li a2,0x1d
8002d2ac:  move a0,v0
8002d2b0:  jal 0x8004483c
8002d2b4:  _addiu a3,s0,0x24
8002d2b8:  lh a0,0x6(s0)
8002d2bc:  li a1,0xc0
8002d2c0:  clear a2
8002d2c4:  li a3,0x40
8002d2c8:  jal 0x80012068
8002d2cc:  _nor a0,zero,a0
8002d2d0:  lw a3,0x0(s0)
8002d2d4:  lui v0,0x10
8002d2d8:  and v0,a3,v0
8002d2dc:  beq v0,zero,0x8002d3c8
8002d2e0:  _nop
8002d2e4:  lw a1,0x8c(s0)
8002d2e8:  nop
8002d2ec:  slti v0,a1,0xbec
8002d2f0:  bne v0,zero,0x8002d3c8
8002d2f4:  _nop
8002d2f8:  lw v0,0x80(s0)
8002d2fc:  nop
8002d300:  bgez v0,0x8002d30c
8002d304:  _nop
8002d308:  addiu v0,v0,0x7f
8002d30c:  lh v1,0x14(s0)
8002d310:  sra v0,v0,0x7
8002d314:  mult v0,v1
8002d318:  lw a0,0x84(s0)
8002d31c:  mflo a2
8002d320:  bgez a0,0x8002d32c
8002d324:  _nop
8002d328:  addiu a0,a0,0x7f
8002d32c:  lh v1,0x1a(s0)
8002d330:  sra v0,a0,0x7
8002d334:  mult v0,v1
8002d338:  lw v0,0x88(s0)
8002d33c:  mflo t0
8002d340:  bgez v0,0x8002d34c
8002d344:  _addu a0,a2,t0
8002d348:  addiu v0,v0,0x7f
8002d34c:  lh v1,0x20(s0)
8002d350:  sra v0,v0,0x7
8002d354:  mult v0,v1
8002d358:  sll v0,a1,0x1
8002d35c:  addu v0,v0,a1
8002d360:  sll v0,v0,0xa
8002d364:  mflo t0
8002d368:  addu v1,a0,t0
8002d36c:  bgez v1,0x8002d378
8002d370:  _nop
8002d374:  subu v1,zero,v1
8002d378:  slt v1,v1,v0
8002d37c:  beq v1,zero,0x8002d3c8
8002d380:  _lui v1,0x8
8002d384:  and v0,a3,v1
8002d388:  bne v0,zero,0x8002d3dc
8002d38c:  _or v0,a3,v1
8002d390:  jal 0x8004410c
8002d394:  _sw v0,0x0(s0)
8002d398:  jal 0x80017160
8002d39c:  _move s2,v0
8002d3a0:  andi v0,v0,0x1
8002d3a4:  beq v0,zero,0x8002d3b0
8002d3a8:  _li a2,0x1a
8002d3ac:  li a2,0x19
8002d3b0:  lw a1,0x5f8(gp)
8002d3b4:  move a0,s2
8002d3b8:  jal 0x8004483c
8002d3bc:  _addiu a3,s0,0x24
8002d3c0:  j 0x8002d3dc
8002d3c4:  _nop
8002d3c8:  lw v0,0x0(s0)
8002d3cc:  lui v1,0xfff7
8002d3d0:  ori v1,v1,0xffff
8002d3d4:  and v0,v0,v1
8002d3d8:  sw v0,0x0(s0)
8002d3dc:  lbu v0,0xd3(s0)
8002d3e0:  nop
8002d3e4:  beq v0,zero,0x8002d434
8002d3e8:  _lui a0,0x1f80
8002d3ec:  lw v1,0x8c(s0)
8002d3f0:  ori a0,a0,0x1bf0
8002d3f4:  sll v0,v0,0x4
8002d3f8:  addu s2,v0,a0
8002d3fc:  srl v0,v1,0x1f
8002d400:  addu v1,v1,v0
8002d404:  sra v1,v1,0x1
8002d408:  slti v0,v1,0x300
8002d40c:  beq v0,zero,0x8002d41c
8002d410:  _li a0,0xc00
8002d414:  j 0x8002d434
8002d418:  _sw zero,0x0(s2)
8002d41c:  slt v0,v1,a0
8002d420:  beq v0,zero,0x8002d42c
8002d424:  _nop
8002d428:  move a0,v1
8002d42c:  sh a0,0x4(s2)
8002d430:  sw s1,0x0(s2)
8002d434:  lw ra,0x1c(sp)
8002d438:  lw s2,0x18(sp)
8002d43c:  lw s1,0x14(sp)
8002d440:  lw s0,0x10(sp)
8002d444:  jr ra
8002d448:  _addiu sp,sp,0x20
