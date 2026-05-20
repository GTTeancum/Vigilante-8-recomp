# addr: 0x8010f1e0  name: FUN_8010f1e0
8010f1e0:  addiu sp,sp,-0x20
8010f1e4:  sw s0,0x10(sp)
8010f1e8:  move s0,a0
8010f1ec:  sw ra,0x18(sp)
8010f1f0:  sw s1,0x14(sp)
8010f1f4:  lw v1,0x0(s0)
8010f1f8:  nop
8010f1fc:  sltiu v0,v1,0x20
8010f200:  beq v0,zero,0x8010f3fc
8010f204:  _sll v0,v1,0x2
8010f208:  lui at,0x8010
8010f20c:  addu at,at,v0
8010f210:  lw v0,0x110c(at)
8010f214:  nop
8010f218:  jr v0
8010f21c:  _nop
8010f220:  lui a0,0x8011
8010f224:  addiu a0,a0,-0x105c
8010f228:  lui at,0x8011
8010f22c:  sw zero,0x3430(at)
8010f230:  lui at,0x8011
8010f234:  sw zero,0x342c(at)
8010f238:  lui at,0x8011
8010f23c:  sltiu a0,zero,0x4300
8010f240:  sw zero,0x3428(at)
8010f244:  li v0,0xa
8010f248:  slti a0,zero,0x3cff
8010f24c:  sw v0,0x0(s0)
8010f250:  lui s1,0x8011
8010f254:  addiu s1,s1,0x343c
8010f258:  lw v1,0x0(s1)
8010f25c:  nop
8010f260:  beq v1,zero,0x8010f3ac
8010f264:  _li v0,0x3
8010f268:  bne v1,v0,0x8010f400
8010f26c:  _li v0,0x1
8010f270:  lw a0,0x8(s1)
8010f274:  lui v0,0x8011
8010f278:  lw v0,0x3434(v0)
8010f27c:  li v1,0x1
8010f280:  lui at,0x8011
8010f284:  sw v1,0x3430(at)
8010f288:  sllv v1,v1,a0
8010f28c:  or v0,v0,v1
8010f290:  lui at,0x8011
8010f294:  sltiu a0,zero,0x4420
8010f298:  sw v0,0x3434(at)
8010f29c:  lw a0,0x8(s1)
8010f2a0:  sltiu a0,zero,0x4224
8010f2a4:  nop
8010f2a8:  li v0,0x15
8010f2ac:  slti a0,zero,0x3cff
8010f2b0:  sw v0,0x0(s0)
8010f2b4:  sltiu a0,zero,0x44dd
8010f2b8:  nop
8010f2bc:  beq v0,zero,0x8010f400
8010f2c0:  _clear v0
8010f2c4:  sltiu a0,zero,0x4498
8010f2c8:  nop
8010f2cc:  li v0,0x1e
8010f2d0:  sw v0,0x0(s0)
8010f2d4:  sltiu a0,zero,0x4420
8010f2d8:  nop
8010f2dc:  lui a0,0x8011
8010f2e0:  lw a0,0x3444(a0)
8010f2e4:  sltiu a0,zero,0x4218
8010f2e8:  nop
8010f2ec:  lw v0,0x0(s0)
8010f2f0:  nop
8010f2f4:  addiu v0,v0,0x1
8010f2f8:  slti a0,zero,0x3cff
8010f2fc:  sw v0,0x0(s0)
8010f300:  sltiu a0,zero,0x44ce
8010f304:  nop
8010f308:  beq v0,zero,0x8010f400
8010f30c:  _clear v0
8010f310:  sltiu a0,zero,0x4462
8010f314:  nop
8010f318:  move v1,v0
8010f31c:  lui at,0x8011
8010f320:  sw v0,0x342c(at)
8010f324:  slti v0,v1,0x3
8010f328:  beq v0,zero,0x8010f348
8010f32c:  _nop
8010f330:  bgtz v1,0x8010f388
8010f334:  _nop
8010f338:  beq v1,zero,0x8010f35c
8010f33c:  _nop
8010f340:  slti a0,zero,0x3cee
8010f344:  nop
8010f348:  li v0,0x4
8010f34c:  beq v1,v0,0x8010f388
8010f350:  _nop
8010f354:  slti a0,zero,0x3cee
8010f358:  nop
8010f35c:  lui v0,0x8011
8010f360:  lw v0,0x3430(v0)
8010f364:  nop
8010f368:  beq v0,zero,0x8010f374
8010f36c:  _clear v1
8010f370:  li v1,0x3
8010f374:  lui v0,0x8011
8010f378:  addiu v0,v0,0x3438
8010f37c:  sw v1,0x4(v0)
8010f380:  slti a0,zero,0x3d00
8010f384:  li v0,0x1
8010f388:  lui v0,0x8011
8010f38c:  lw v0,0x3428(v0)
8010f390:  nop
8010f394:  addiu v0,v0,0x1
8010f398:  lui at,0x8011
8010f39c:  sw v0,0x3428(at)
8010f3a0:  slti v0,v0,0x5
8010f3a4:  beq v0,zero,0x8010f3b8
8010f3a8:  _nop
8010f3ac:  li v0,0x1e
8010f3b0:  slti a0,zero,0x3cff
8010f3b4:  sw v0,0x0(s0)
8010f3b8:  lui a0,0x8011
8010f3bc:  lw a0,0x342c(a0)
8010f3c0:  li v0,0x4
8010f3c4:  bne a0,v0,0x8010f3e0
8010f3c8:  _nop
8010f3cc:  lui v0,0x8011
8010f3d0:  addiu v0,v0,0x3438
8010f3d4:  sw a0,0x4(v0)
8010f3d8:  slti a0,zero,0x3d00
8010f3dc:  li v0,0x1
8010f3e0:  sltiu a0,zero,0x41c5
8010f3e4:  nop
8010f3e8:  lui v1,0x8011
8010f3ec:  addiu v1,v1,0x3438
8010f3f0:  sw v0,0x4(v1)
8010f3f4:  slti a0,zero,0x3d00
8010f3f8:  li v0,0x1
8010f3fc:  clear v0
8010f400:  lw ra,0x18(sp)
8010f404:  lw s1,0x14(sp)
8010f408:  lw s0,0x10(sp)
8010f40c:  jr ra
8010f410:  _addiu sp,sp,0x20
