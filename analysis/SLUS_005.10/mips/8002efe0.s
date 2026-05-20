# addr: 0x8002efe0  name: FUN_8002efe0
8002efe0:  addiu sp,sp,-0x50
8002efe4:  sw s4,0x40(sp)
8002efe8:  move s4,a1
8002efec:  sw ra,0x48(sp)
8002eff0:  sw s5,0x44(sp)
8002eff4:  sw s3,0x3c(sp)
8002eff8:  sw s2,0x38(sp)
8002effc:  sw s1,0x34(sp)
8002f000:  sw s0,0x30(sp)
8002f004:  lh v1,0x0(s4)
8002f008:  move s2,a0
8002f00c:  li v0,0x3
8002f010:  lw s3,0x8(s4)
8002f014:  beq v1,v0,0x8002f46c
8002f018:  _addiu s0,s2,0xa4
8002f01c:  slti v0,v1,0x4
8002f020:  beq v0,zero,0x8002f038
8002f024:  _li v0,0x2
8002f028:  beq v1,v0,0x8002f04c
8002f02c:  _nop
8002f030:  j 0x8002f974
8002f034:  _nop
8002f038:  slti v0,v1,0x6
8002f03c:  beq v0,zero,0x8002f974
8002f040:  _nop
8002f044:  j 0x8002f654
8002f048:  _nop
8002f04c:  lb v0,0xe(s0)
8002f050:  nop
8002f054:  bltz v0,0x8002f064
8002f058:  _li s5,0x100
8002f05c:  j 0x8002f06c
8002f060:  _li v0,0x200
8002f064:  li s5,0x200
8002f068:  li v0,0x100
8002f06c:  and v0,s3,v0
8002f070:  beq v0,zero,0x8002f0b4
8002f074:  _sll v1,s5,0x10
8002f078:  lw v0,0x8c(s2)
8002f07c:  nop
8002f080:  slti v0,v0,0x1da
8002f084:  beq v0,zero,0x8002f0a8
8002f088:  _nop
8002f08c:  lb v0,0xe(s0)
8002f090:  nop
8002f094:  bltz v0,0x8002f0a0
8002f098:  _li v1,0x1
8002f09c:  li v1,-0x1
8002f0a0:  j 0x8002f134
8002f0a4:  _sb v1,0xe(s0)
8002f0a8:  lhu v0,0x8(s0)
8002f0ac:  j 0x8002f130
8002f0b0:  _subu v0,zero,v0
8002f0b4:  lw v0,0x0(s2)
8002f0b8:  or v1,s5,v1
8002f0bc:  and s1,s3,v1
8002f0c0:  lui v1,0x800
8002f0c4:  and v0,v0,v1
8002f0c8:  beq v0,zero,0x8002f0e0
8002f0cc:  _move a0,s2
8002f0d0:  jal 0x8002c59c
8002f0d4:  _move a1,s1
8002f0d8:  j 0x8002f138
8002f0dc:  _andi v0,s3,0x1800
8002f0e0:  lw v0,0xc(s4)
8002f0e4:  lui v1,0xf000
8002f0e8:  and v0,v0,v1
8002f0ec:  beq v0,zero,0x8002f0f8
8002f0f0:  _move a1,s1
8002f0f4:  andi a1,s1,0xffff
8002f0f8:  jal 0x8002ee94
8002f0fc:  _move a0,s2
8002f100:  andi v0,s1,0xffff
8002f104:  beq v0,zero,0x8002f118
8002f108:  _nop
8002f10c:  lhu v0,0x8(s0)
8002f110:  j 0x8002f134
8002f114:  _sh v0,0x2(s0)
8002f118:  lh v0,0x2(s0)
8002f11c:  lhu v1,0x2(s0)
8002f120:  bgez v0,0x8002f130
8002f124:  _addiu v0,v1,-0x2
8002f128:  j 0x8002f134
8002f12c:  _sh zero,0x2(s0)
8002f130:  sh v0,0x2(s0)
8002f134:  andi v0,s3,0x1800
8002f138:  beq v0,zero,0x8002f418
8002f13c:  _nop
8002f140:  lh v0,0x18(s2)
8002f144:  nop
8002f148:  blez v0,0x8002f3d8
8002f14c:  _lui v0,0x800
8002f150:  lb a0,0xe(s0)
8002f154:  nop
8002f158:  blez a0,0x8002f178
8002f15c:  _clear a2
8002f160:  lw v0,0x0(s2)
8002f164:  lui v1,0xbfff
8002f168:  ori v1,v1,0xffff
8002f16c:  and v0,v0,v1
8002f170:  j 0x8002f214
8002f174:  _sw v0,0x0(s2)
8002f178:  lui v0,0x1800
8002f17c:  and v0,s3,v0
8002f180:  beq v0,zero,0x8002f204
8002f184:  _lui v0,0xbfff
8002f188:  lw v1,0x0(s2)
8002f18c:  ori v0,v0,0xffff
8002f190:  bgez a0,0x8002f200
8002f194:  _and a1,v1,v0
8002f198:  and v0,s3,s5
8002f19c:  bne v0,zero,0x8002f1fc
8002f1a0:  _lui v0,0x4000
8002f1a4:  lw v0,0x80(s2)
8002f1a8:  lh a0,0x14(s2)
8002f1ac:  bgez v0,0x8002f1b8
8002f1b0:  _nop
8002f1b4:  addiu v0,v0,0x7f
8002f1b8:  sra v0,v0,0x7
8002f1bc:  mult a0,v0
8002f1c0:  lw v1,0x88(s2)
8002f1c4:  lh a0,0x20(s2)
8002f1c8:  mflo a2
8002f1cc:  bgez v1,0x8002f1dc
8002f1d0:  _sra v0,v1,0x7
8002f1d4:  addiu v1,v1,0x7f
8002f1d8:  sra v0,v1,0x7
8002f1dc:  mult a0,v0
8002f1e0:  lui v1,0xffb3
8002f1e4:  ori v1,v1,0xc000
8002f1e8:  mflo a3
8002f1ec:  addu v0,a2,a3
8002f1f0:  slt v0,v0,v1
8002f1f4:  beq v0,zero,0x8002f200
8002f1f8:  _lui v0,0x4000
8002f1fc:  or a1,a1,v0
8002f200:  sw a1,0x0(s2)
8002f204:  lw v0,0x0(s2)
8002f208:  nop
8002f20c:  srl a2,v0,0x1e
8002f210:  andi a2,a2,0x1
8002f214:  andi v0,s3,0x400
8002f218:  beq v0,zero,0x8002f290
8002f21c:  _andi v0,s3,0x800
8002f220:  beq v0,zero,0x8002f25c
8002f224:  _li v1,-0x2aa
8002f228:  lh v0,0x0(s0)
8002f22c:  nop
8002f230:  addiu a0,v0,-0x20
8002f234:  slt v0,v1,a0
8002f238:  beq v0,zero,0x8002f244
8002f23c:  _nop
8002f240:  move v1,a0
8002f244:  sh v1,0x0(s0)
8002f248:  lw v0,0x94(s2)
8002f24c:  beq a2,zero,0x8002f288
8002f250:  _nop
8002f254:  j 0x8002f970
8002f258:  _addiu v0,v0,0x500
8002f25c:  lh v0,0x0(s0)
8002f260:  li a0,0x2aa
8002f264:  addiu v1,v0,0x20
8002f268:  slt v0,v1,a0
8002f26c:  beq v0,zero,0x8002f278
8002f270:  _nop
8002f274:  move a0,v1
8002f278:  sh a0,0x0(s0)
8002f27c:  lw v0,0x94(s2)
8002f280:  beq a2,zero,0x8002f254
8002f284:  _nop
8002f288:  j 0x8002f970
8002f28c:  _addiu v0,v0,-0x500
8002f290:  sw zero,0x28(sp)
8002f294:  sw zero,0x24(sp)
8002f298:  sw zero,0x14(sp)
8002f29c:  beq v0,zero,0x8002f358
8002f2a0:  _sw zero,0x10(sp)
8002f2a4:  lh v1,0x0(s0)
8002f2a8:  clear v0
8002f2ac:  bgez v1,0x8002f2b8
8002f2b0:  _addiu a0,v1,-0x10
8002f2b4:  move v0,v1
8002f2b8:  bgez v0,0x8002f2c4
8002f2bc:  _li v1,-0x2aa
8002f2c0:  addiu v0,v0,0x3f
8002f2c4:  sra v0,v0,0x6
8002f2c8:  subu a0,a0,v0
8002f2cc:  slt v0,v1,a0
8002f2d0:  beq v0,zero,0x8002f2e0
8002f2d4:  _sll v0,v1,0x10
8002f2d8:  move v1,a0
8002f2dc:  sll v0,v1,0x10
8002f2e0:  sra a1,v0,0x10
8002f2e4:  bgez a1,0x8002f974
8002f2e8:  _sh v1,0x0(s0)
8002f2ec:  lh v1,0x6(s0)
8002f2f0:  lw v0,0x8c(s2)
8002f2f4:  nop
8002f2f8:  mult v0,v1
8002f2fc:  mflo a0
8002f300:  sll v0,a2,0x1
8002f304:  addiu v0,v0,-0x1
8002f308:  mult v0,a1
8002f30c:  lh a1,0x4(s0)
8002f310:  mflo a2
8002f314:  bgez a0,0x8002f324
8002f318:  _sra v0,a0,0xc
8002f31c:  addiu a0,a0,0xfff
8002f320:  sra v0,a0,0xc
8002f324:  addu v0,a1,v0
8002f328:  blez v0,0x8002f334
8002f32c:  _clear v1
8002f330:  move v1,v0
8002f334:  mult a2,v1
8002f338:  mflo v1
8002f33c:  bgez v1,0x8002f348
8002f340:  _nop
8002f344:  addiu v1,v1,0xf
8002f348:  lw v0,0x94(s2)
8002f34c:  sra v1,v1,0x4
8002f350:  j 0x8002f970
8002f354:  _subu v0,v0,v1
8002f358:  lh v1,0x0(s0)
8002f35c:  clear v0
8002f360:  blez v1,0x8002f36c
8002f364:  _addiu a0,v1,0x10
8002f368:  move v0,v1
8002f36c:  bgez v0,0x8002f378
8002f370:  _li v1,0x2aa
8002f374:  addiu v0,v0,0x3f
8002f378:  sra v0,v0,0x6
8002f37c:  subu a0,a0,v0
8002f380:  slt v0,a0,v1
8002f384:  beq v0,zero,0x8002f394
8002f388:  _sll v0,v1,0x10
8002f38c:  move v1,a0
8002f390:  sll v0,v1,0x10
8002f394:  sra a1,v0,0x10
8002f398:  blez a1,0x8002f974
8002f39c:  _sh v1,0x0(s0)
8002f3a0:  lh v1,0x6(s0)
8002f3a4:  lw v0,0x8c(s2)
8002f3a8:  nop
8002f3ac:  mult v0,v1
8002f3b0:  mflo a0
8002f3b4:  sll v0,a2,0x1
8002f3b8:  addiu v0,v0,-0x1
8002f3bc:  mult v0,a1
8002f3c0:  lh a1,0x4(s0)
8002f3c4:  mflo a2
8002f3c8:  bgez a0,0x8002f324
8002f3cc:  _sra v0,a0,0xc
8002f3d0:  j 0x8002f320
8002f3d4:  _addiu a0,a0,0xfff
8002f3d8:  and v0,s3,v0
8002f3dc:  beq v0,zero,0x8002f3f8
8002f3e0:  _lui v0,0x1000
8002f3e4:  lw v0,0x98(s2)
8002f3e8:  nop
8002f3ec:  addiu v0,v0,-0x4000
8002f3f0:  j 0x8002f974
8002f3f4:  _sw v0,0x98(s2)
8002f3f8:  and v0,s3,v0
8002f3fc:  beq v0,zero,0x8002f974
8002f400:  _nop
8002f404:  lw v0,0x98(s2)
8002f408:  nop
8002f40c:  addiu v0,v0,0x4000
8002f410:  j 0x8002f974
8002f414:  _sw v0,0x98(s2)
8002f418:  lh v1,0x0(s0)
8002f41c:  lw v0,0x8c(s2)
8002f420:  nop
8002f424:  mult v1,v0
8002f428:  lhu v1,0x0(s0)
8002f42c:  mflo v0
8002f430:  bgez v0,0x8002f43c
8002f434:  _nop
8002f438:  addiu v0,v0,0x7fff
8002f43c:  sra v0,v0,0xf
8002f440:  subu v0,v1,v0
8002f444:  sh v0,0x0(s0)
8002f448:  andi v0,s3,0x400
8002f44c:  beq v0,zero,0x8002f974
8002f450:  _nop
8002f454:  lhu v0,0x8(s0)
8002f458:  nop
8002f45c:  subu v0,zero,v0
8002f460:  sll v0,v0,0x1
8002f464:  j 0x8002f974
8002f468:  _sh v0,0x2(s0)
8002f46c:  lbu v1,0x10(s4)
8002f470:  nop
8002f474:  addiu v1,v1,-0x80
8002f478:  sll v0,v1,0x2
8002f47c:  addu v0,v0,v1
8002f480:  sh v0,0xa4(s2)
8002f484:  lbu a0,0x11(s4)
8002f488:  lb v1,0x15(s4)
8002f48c:  sltiu v0,a0,0x81
8002f490:  bltz v1,0x8002f4a0
8002f494:  _xori s1,v0,0x1
8002f498:  lui v0,0x1
8002f49c:  or s1,s1,v0
8002f4a0:  lw v0,0x0(s2)
8002f4a4:  lui v1,0x800
8002f4a8:  and v0,v0,v1
8002f4ac:  beq v0,zero,0x8002f4c8
8002f4b0:  _nop
8002f4b4:  move a0,s2
8002f4b8:  jal 0x8002c59c
8002f4bc:  _move a1,s1
8002f4c0:  j 0x8002f580
8002f4c4:  _nop
8002f4c8:  lb v0,0xe(s0)
8002f4cc:  nop
8002f4d0:  bgez v0,0x8002f4ec
8002f4d4:  _andi v0,s3,0x100
8002f4d8:  sltiu v0,a0,0x11
8002f4dc:  bne v0,zero,0x8002f4ec
8002f4e0:  _andi v0,s3,0x100
8002f4e4:  j 0x8002f4f4
8002f4e8:  _li v0,0x1
8002f4ec:  beq v0,zero,0x8002f4f8
8002f4f0:  _li v0,-0x1
8002f4f4:  sb v0,0xe(s0)
8002f4f8:  lb v0,0xe(s0)
8002f4fc:  lhu v1,0x8(s0)
8002f500:  bgez v0,0x8002f518
8002f504:  _andi v0,s3,0x100
8002f508:  bne v0,zero,0x8002f570
8002f50c:  _nop
8002f510:  j 0x8002f570
8002f514:  _clear v1
8002f518:  lbu v0,0x12(s4)
8002f51c:  lbu a0,0x11(s4)
8002f520:  sltiu v0,v0,0xf1
8002f524:  bne v0,zero,0x8002f54c
8002f528:  _nop
8002f52c:  lh v0,0x0(s0)
8002f530:  nop
8002f534:  bgez v0,0x8002f540
8002f538:  _nop
8002f53c:  subu v0,zero,v0
8002f540:  slti v0,v0,0xaa
8002f544:  beq v0,zero,0x8002f55c
8002f548:  _mult a0,v1
8002f54c:  lbu v0,0x12(s4)
8002f550:  nop
8002f554:  subu v0,a0,v0
8002f558:  mult v0,v1
8002f55c:  mflo v0
8002f560:  bgez v0,0x8002f570
8002f564:  _srl v1,v0,0x8
8002f568:  addiu v0,v0,0xff
8002f56c:  srl v1,v0,0x8
8002f570:  sh v1,0x2(s0)
8002f574:  move a0,s2
8002f578:  jal 0x8002ee94
8002f57c:  _move a1,s1
8002f580:  lh v0,0x18(s2)
8002f584:  nop
8002f588:  bltz v0,0x8002f88c
8002f58c:  _nop
8002f590:  lbu v0,0x12(s4)
8002f594:  nop
8002f598:  sltiu v0,v0,0xf1
8002f59c:  beq v0,zero,0x8002f5ac
8002f5a0:  _andi v1,s3,0x400
8002f5a4:  beq v1,zero,0x8002f5fc
8002f5a8:  _nop
8002f5ac:  lh v0,0x0(s0)
8002f5b0:  nop
8002f5b4:  bgez v0,0x8002f5c0
8002f5b8:  _nop
8002f5bc:  subu v0,zero,v0
8002f5c0:  slti v0,v0,0xaa
8002f5c4:  beq v0,zero,0x8002f5e0
8002f5c8:  _nop
8002f5cc:  lhu v0,0x8(s0)
8002f5d0:  nop
8002f5d4:  subu v0,zero,v0
8002f5d8:  sll v0,v0,0x1
8002f5dc:  sh v0,0x2(s0)
8002f5e0:  lh v1,0x0(s0)
8002f5e4:  lb v0,0xe(s0)
8002f5e8:  lw a0,0x94(s2)
8002f5ec:  bgez v0,0x8002f96c
8002f5f0:  _sll v0,v1,0x1
8002f5f4:  j 0x8002f970
8002f5f8:  _subu v0,a0,v0
8002f5fc:  lh v1,0x6(s0)
8002f600:  lw v0,0x8c(s2)
8002f604:  nop
8002f608:  mult v0,v1
8002f60c:  lh a1,0x0(s0)
8002f610:  lh a0,0x4(s0)
8002f614:  mflo v0
8002f618:  bgez v0,0x8002f624
8002f61c:  _nop
8002f620:  addiu v0,v0,0xfff
8002f624:  sra v0,v0,0xc
8002f628:  addu v0,a0,v0
8002f62c:  blez v0,0x8002f638
8002f630:  _clear v1
8002f634:  move v1,v0
8002f638:  lb v0,0xe(s0)
8002f63c:  lw a0,0x94(s2)
8002f640:  bgez v0,0x8002f958
8002f644:  _mult a1,v1
8002f648:  subu v0,zero,v1
8002f64c:  j 0x8002f958
8002f650:  _mult a1,v0
8002f654:  lbu a1,0x12(s4)
8002f658:  nop
8002f65c:  addiu v1,a1,-0x80
8002f660:  bgez v1,0x8002f66c
8002f664:  _move v0,v1
8002f668:  subu v0,zero,v0
8002f66c:  slti v0,v0,0x21
8002f670:  bne v0,zero,0x8002f698
8002f674:  _nop
8002f678:  lw a0,0xe0(s2)
8002f67c:  bgez v1,0x8002f688
8002f680:  _nop
8002f684:  addiu v1,a1,-0x7d
8002f688:  lhu v0,0x8e(a0)
8002f68c:  sra v1,v1,0x2
8002f690:  subu v0,v0,v1
8002f694:  sh v0,0x8e(a0)
8002f698:  lbu v0,0x13(s4)
8002f69c:  nop
8002f6a0:  addiu v1,v0,-0x80
8002f6a4:  bgez v1,0x8002f6b0
8002f6a8:  _move v0,v1
8002f6ac:  subu v0,zero,v0
8002f6b0:  slti v0,v0,0x21
8002f6b4:  bne v0,zero,0x8002f724
8002f6b8:  _sll v0,v1,0x1
8002f6bc:  addu v0,v0,v1
8002f6c0:  sll v0,v0,0x6
8002f6c4:  subu v0,v0,v1
8002f6c8:  sll v0,v0,0x2
8002f6cc:  subu v0,v0,v1
8002f6d0:  sll v0,v0,0x2
8002f6d4:  subu v0,v0,v1
8002f6d8:  lw a1,0xe0(s2)
8002f6dc:  bgez v0,0x8002f6e8
8002f6e0:  _nop
8002f6e4:  addiu v0,v0,0x7f
8002f6e8:  lw v1,0x94(a1)
8002f6ec:  lw a0,0x54(s2)
8002f6f0:  sra v0,v0,0x7
8002f6f4:  addu v1,v1,v0
8002f6f8:  sll a0,a0,0x1
8002f6fc:  slt v0,v1,a0
8002f700:  bne v0,zero,0x8002f720
8002f704:  _nop
8002f708:  lui a0,0x14
8002f70c:  move v0,a0
8002f710:  slt v0,v0,v1
8002f714:  bne v0,zero,0x8002f720
8002f718:  _nop
8002f71c:  move a0,v1
8002f720:  sw a0,0x94(a1)
8002f724:  lbu v0,0x10(s4)
8002f728:  nop
8002f72c:  addiu v0,v0,-0x80
8002f730:  bgez v0,0x8002f73c
8002f734:  _move v1,v0
8002f738:  subu v1,zero,v1
8002f73c:  mult v1,v0
8002f740:  mflo v1
8002f744:  lui v0,0x2aaa
8002f748:  ori v0,v0,0xaaab
8002f74c:  mult v1,v0
8002f750:  sra v1,v1,0x1f
8002f754:  mfhi v0
8002f758:  sra v0,v0,0x2
8002f75c:  subu v0,v0,v1
8002f760:  sh v0,0x0(s0)
8002f764:  lbu a0,0x11(s4)
8002f768:  nop
8002f76c:  sltiu v0,a0,0x41
8002f770:  beq v0,zero,0x8002f788
8002f774:  _clear v1
8002f778:  lbu v0,0x15(s4)
8002f77c:  nop
8002f780:  sltiu v0,v0,0x41
8002f784:  xori v1,v0,0x1
8002f788:  lui v0,0x100
8002f78c:  ori v0,v0,0x100
8002f790:  and a1,s3,v0
8002f794:  sltiu v0,a0,0x40
8002f798:  beq v0,zero,0x8002f7ac
8002f79c:  _sll v1,v1,0x10
8002f7a0:  ori v0,a1,0x1
8002f7a4:  j 0x8002f7b0
8002f7a8:  _or s1,v1,v0
8002f7ac:  or s1,v1,a1
8002f7b0:  lw v0,0x0(s2)
8002f7b4:  lui v1,0x800
8002f7b8:  and v0,v0,v1
8002f7bc:  beq v0,zero,0x8002f7d4
8002f7c0:  _move a0,s2
8002f7c4:  jal 0x8002c59c
8002f7c8:  _move a1,s1
8002f7cc:  j 0x8002f87c
8002f7d0:  _nop
8002f7d4:  lb v0,0xe(s0)
8002f7d8:  nop
8002f7dc:  bltz v0,0x8002f7e8
8002f7e0:  _li a1,-0x1
8002f7e4:  li a1,0x1
8002f7e8:  andi v0,s3,0x100
8002f7ec:  beq v0,zero,0x8002f800
8002f7f0:  _li v0,0x80
8002f7f4:  lhu v0,0x8(s0)
8002f7f8:  j 0x8002f828
8002f7fc:  _mult a1,v0
8002f800:  lbu v1,0x11(s4)
8002f804:  lhu a0,0x8(s0)
8002f808:  subu v0,v0,v1
8002f80c:  mult v0,a0
8002f810:  mflo v0
8002f814:  bgez v0,0x8002f820
8002f818:  _nop
8002f81c:  addiu v0,v0,0x7f
8002f820:  sra v0,v0,0x7
8002f824:  mult a1,v0
8002f828:  mflo a3
8002f82c:  sh a3,0x2(s0)
8002f830:  lw v0,0x8c(s2)
8002f834:  nop
8002f838:  slti v0,v0,0x1da
8002f83c:  beq v0,zero,0x8002f874
8002f840:  _move a0,s2
8002f844:  lh v0,0x2(s0)
8002f848:  nop
8002f84c:  slti v0,v0,-0x10
8002f850:  beq v0,zero,0x8002f874
8002f854:  _nop
8002f858:  lb v0,0xe(s0)
8002f85c:  nop
8002f860:  bltz v0,0x8002f86c
8002f864:  _li v1,0x1
8002f868:  li v1,-0x1
8002f86c:  sb v1,0xe(s0)
8002f870:  move a0,s2
8002f874:  jal 0x8002ee94
8002f878:  _move a1,s1
8002f87c:  lh v0,0x18(s2)
8002f880:  nop
8002f884:  bgez v0,0x8002f8ac
8002f888:  _andi v0,s3,0x400
8002f88c:  lbu v0,0x10(s4)
8002f890:  lbu a0,0x14(s4)
8002f894:  lw v1,0x98(s2)
8002f898:  subu v0,v0,a0
8002f89c:  sll v0,v0,0x8
8002f8a0:  addu v1,v1,v0
8002f8a4:  j 0x8002f974
8002f8a8:  _sw v1,0x98(s2)
8002f8ac:  beq v0,zero,0x8002f904
8002f8b0:  _nop
8002f8b4:  lh v0,0x0(s0)
8002f8b8:  nop
8002f8bc:  bgez v0,0x8002f8c8
8002f8c0:  _nop
8002f8c4:  subu v0,zero,v0
8002f8c8:  slti v0,v0,0xaa
8002f8cc:  beq v0,zero,0x8002f8e8
8002f8d0:  _nop
8002f8d4:  lhu v0,0x8(s0)
8002f8d8:  nop
8002f8dc:  subu v0,zero,v0
8002f8e0:  sll v0,v0,0x1
8002f8e4:  sh v0,0x2(s0)
8002f8e8:  lh v1,0x0(s0)
8002f8ec:  lb v0,0xe(s0)
8002f8f0:  lw a0,0x94(s2)
8002f8f4:  bltz v0,0x8002f5f4
8002f8f8:  _sll v0,v1,0x1
8002f8fc:  j 0x8002f970
8002f900:  _addu v0,a0,v0
8002f904:  lh v1,0x6(s0)
8002f908:  lw v0,0x8c(s2)
8002f90c:  nop
8002f910:  mult v0,v1
8002f914:  lh a1,0x0(s0)
8002f918:  lh a0,0x4(s0)
8002f91c:  mflo v0
8002f920:  bgez v0,0x8002f92c
8002f924:  _nop
8002f928:  addiu v0,v0,0xfff
8002f92c:  sra v0,v0,0xc
8002f930:  addu v0,a0,v0
8002f934:  blez v0,0x8002f940
8002f938:  _clear v1
8002f93c:  move v1,v0
8002f940:  lb v0,0xe(s0)
8002f944:  lw a0,0x94(s2)
8002f948:  bgez v0,0x8002f958
8002f94c:  _mult a1,v1
8002f950:  subu v0,zero,v1
8002f954:  mult a1,v0
8002f958:  mflo v0
8002f95c:  bgez v0,0x8002f968
8002f960:  _nop
8002f964:  addiu v0,v0,0xf
8002f968:  sra v0,v0,0x4
8002f96c:  addu v0,a0,v0
8002f970:  sw v0,0x94(s2)
8002f974:  lw ra,0x48(sp)
8002f978:  lw s5,0x44(sp)
8002f97c:  lw s4,0x40(sp)
8002f980:  lw s3,0x3c(sp)
8002f984:  lw s2,0x38(sp)
8002f988:  lw s1,0x34(sp)
8002f98c:  lw s0,0x30(sp)
8002f990:  jr ra
8002f994:  _addiu sp,sp,0x50
