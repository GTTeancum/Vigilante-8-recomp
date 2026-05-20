# addr: 0x80053394  name: SPRINTF_OBJ_390
80053394:  lw v1,0x210(sp)
80053398:  nop
8005339c:  srl v0,v1,0x4
800533a0:  andi v0,v0,0x1
800533a4:  bne v0,zero,0x800533e8
800533a8:  _srl v0,v1,0x3
800533ac:  andi v0,v0,0x1
800533b0:  beq v0,zero,0x800533d4
800533b4:  _nop
800533b8:  lw v1,0x214(sp)
800533bc:  lb v0,0x211(sp)
800533c0:  nop
800533c4:  beq v0,zero,0x800533d4
800533c8:  _sw v1,0x218(sp)
800533cc:  addiu v0,v1,-0x1
800533d0:  sw v0,0x218(sp)
800533d4:  lw v0,0x218(sp)
800533d8:  nop
800533dc:  bgtz v0,0x800533e8
800533e0:  _li v0,0x1
800533e4:  sw v0,0x218(sp)
800533e8:  beq a0,zero,0x8005342c
800533ec:  _clear s0
800533f0:  lui a1,0xcccc
800533f4:  ori a1,a1,0xcccd
800533f8:  multu a0,a1
800533fc:  addiu s1,s1,-0x1
80053400:  addiu s0,s0,0x1
80053404:  mfhi t0
80053408:  srl v1,t0,0x3
8005340c:  sll v0,v1,0x2
80053410:  addu v0,v0,v1
80053414:  sll v0,v0,0x1
80053418:  subu v0,a0,v0
8005341c:  addiu v0,v0,0x30
80053420:  move a0,v1
80053424:  bne a0,zero,0x800533f8
80053428:  _sb v0,0x0(s1)
8005342c:  lw v0,0x218(sp)
80053430:  nop
80053434:  slt v0,s0,v0
80053438:  beq v0,zero,0x80053460
8005343c:  _li v1,0x30
80053440:  addiu s1,s1,-0x1
80053444:  sb v1,0x0(s1)
80053448:  lw v0,0x218(sp)
8005344c:  addiu s0,s0,0x1
80053450:  slt v0,s0,v0
80053454:  bne v0,zero,0x80053444
80053458:  _addiu s1,s1,-0x1
8005345c:  addiu s1,s1,0x1
80053460:  lb v0,0x211(sp)
80053464:  lbu v1,0x211(sp)
80053468:  beq v0,zero,0x800537a4
8005346c:  _nop
80053470:  addiu s1,s1,-0x1
80053474:  sb v1,0x0(s1)
80053478:  j 0x800537a4
8005347c:  _addiu s0,s0,0x1
