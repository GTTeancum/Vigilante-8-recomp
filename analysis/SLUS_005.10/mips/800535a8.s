# addr: 0x800535a8  name: SPRINTF_OBJ_5A4
800535a8:  lw v0,0x220(sp)
800535ac:  nop
800535b0:  lw a0,0x0(v0)
800535b4:  addiu v0,v0,0x4
800535b8:  sw v0,0x220(sp)
800535bc:  lw v1,0x210(sp)
800535c0:  nop
800535c4:  srl v0,v1,0x5
800535c8:  andi v0,v0,0x1
800535cc:  beq v0,zero,0x800535d8
800535d0:  _srl v0,v1,0x4
800535d4:  andi a0,a0,0xffff
800535d8:  andi v0,v0,0x1
800535dc:  bne v0,zero,0x8005361c
800535e0:  _srl v0,v1,0x3
800535e4:  andi v0,v0,0x1
800535e8:  beq v0,zero,0x80053608
800535ec:  _srl v0,v1,0x2
800535f0:  lw a2,0x214(sp)
800535f4:  andi v0,v0,0x1
800535f8:  beq v0,zero,0x80053608
800535fc:  _sw a2,0x218(sp)
80053600:  addiu v0,a2,-0x2
80053604:  sw v0,0x218(sp)
80053608:  lw v0,0x218(sp)
8005360c:  nop
80053610:  bgtz v0,0x8005361c
80053614:  _li v0,0x1
80053618:  sw v0,0x218(sp)
8005361c:  beq a0,zero,0x80053644
80053620:  _clear s0
80053624:  addiu s1,s1,-0x1
80053628:  andi v0,a0,0xf
8005362c:  srl a0,a0,0x4
80053630:  addu v0,a3,v0
80053634:  lbu v0,0x0(v0)
80053638:  addiu s0,s0,0x1
8005363c:  bne a0,zero,0x80053624
80053640:  _sb v0,0x0(s1)
80053644:  lw v0,0x218(sp)
80053648:  nop
8005364c:  slt v0,s0,v0
80053650:  beq v0,zero,0x8005367c
80053654:  _nop
80053658:  li v1,0x30
8005365c:  addiu s1,s1,-0x1
80053660:  sb v1,0x0(s1)
80053664:  lw v0,0x218(sp)
80053668:  addiu s0,s0,0x1
8005366c:  slt v0,s0,v0
80053670:  bne v0,zero,0x80053660
80053674:  _addiu s1,s1,-0x1
80053678:  addiu s1,s1,0x1
8005367c:  lw v0,0x210(sp)
80053680:  nop
80053684:  srl v0,v0,0x2
80053688:  andi v0,v0,0x1
8005368c:  beq v0,zero,0x800537a4
80053690:  _li v0,0x30
80053694:  addiu s1,s1,-0x1
80053698:  sb a1,0x0(s1)
8005369c:  addiu s1,s1,-0x1
800536a0:  addiu s0,s0,0x2
800536a4:  j 0x800537a4
800536a8:  _sb v0,0x0(s1)
