# addr: 0x80017594  name: FUN_80017594
80017594:  addiu sp,sp,-0x30
80017598:  sw s2,0x28(sp)
8001759c:  move s2,a0
800175a0:  sw s1,0x24(sp)
800175a4:  move s1,a1
800175a8:  sw s0,0x20(sp)
800175ac:  move s0,a2
800175b0:  addiu a0,s2,0x10
800175b4:  sw ra,0x2c(sp)
800175b8:  jal 0x80043358
800175bc:  _addiu a2,sp,0x10
800175c0:  lw a0,0x0(s0)
800175c4:  lw v1,0x4(s0)
800175c8:  lw v0,0x8(s0)
800175cc:  sra a0,a0,0x4
800175d0:  sra v1,v1,0x4
800175d4:  sra v0,v0,0x4
800175d8:  gte_ldR11R12 a0
800175dc:  gte_ldR22R23 v1
800175e0:  gte_ldR33 v0
800175e4:  lw a0,0x0(s1)
800175e8:  lw v1,0x4(s1)
800175ec:  lw v0,0x8(s1)
800175f0:  sra a0,a0,0x3
800175f4:  sra v1,v1,0x3
800175f8:  sra v0,v0,0x3
800175fc:  ldsv_ a0,v1,v0
80017608:  nOP12
80017614:  lw v0,0x80(s2)
80017618:  lw v1,0x10(sp)
8001761c:  addiu a0,s2,0x80
80017620:  addu v0,v0,v1
80017624:  sw v0,0x80(s2)
80017628:  lw v0,0x4(a0)
8001762c:  addiu a2,sp,0x10
80017630:  lw a1,0x4(a2)
80017634:  lw v1,0x8(a0)
80017638:  addu v0,v0,a1
8001763c:  sw v0,0x4(a0)
80017640:  lw v0,0x8(a2)
80017644:  nop
80017648:  addu v1,v1,v0
8001764c:  sw v1,0x8(a0)
80017650:  gte_stMAC1 v1
80017654:  lh v0,0x9c(s2)
80017658:  nop
8001765c:  mult v1,v0
80017660:  mflo v1
80017664:  bgez v1,0x80017670
80017668:  _nop
8001766c:  addiu v1,v1,0x3f
80017670:  lw v0,0x90(s2)
80017674:  sra v1,v1,0x6
80017678:  addu v0,v0,v1
8001767c:  sw v0,0x90(s2)
80017680:  gte_stMAC2 v1
80017684:  lh v0,0x9e(s2)
80017688:  nop
8001768c:  mult v1,v0
80017690:  mflo v1
80017694:  bgez v1,0x800176a0
80017698:  _nop
8001769c:  addiu v1,v1,0x3f
800176a0:  lw v0,0x94(s2)
800176a4:  sra v1,v1,0x6
800176a8:  addu v0,v0,v1
800176ac:  sw v0,0x94(s2)
800176b0:  gte_stMAC3 v1
800176b4:  lh v0,0xa0(s2)
800176b8:  nop
800176bc:  mult v1,v0
800176c0:  mflo v1
800176c4:  bgez v1,0x800176d0
800176c8:  _nop
800176cc:  addiu v1,v1,0x3f
800176d0:  lw v0,0x98(s2)
800176d4:  sra v1,v1,0x6
800176d8:  addu v0,v0,v1
800176dc:  sw v0,0x98(s2)
800176e0:  lw ra,0x2c(sp)
800176e4:  lw s2,0x28(sp)
800176e8:  lw s1,0x24(sp)
800176ec:  lw s0,0x20(sp)
800176f0:  jr ra
800176f4:  _addiu sp,sp,0x30
