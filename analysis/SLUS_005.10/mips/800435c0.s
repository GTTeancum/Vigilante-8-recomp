# addr: 0x800435c0  name: FUN_800435c0
800435c0:  move t7,ra
800435c4:  jal 0x8004366c
800435c8:  _nop
800435cc:  lw t0,0x0(a1)
800435d0:  lw t1,0x4(a1)
800435d4:  lw t2,0x8(a1)
800435d8:  lw t3,0x14(a0)
800435dc:  lw t4,0x18(a0)
800435e0:  lw t5,0x1c(a0)
800435e4:  sub t0,t0,t3
800435e8:  sub t1,t1,t4
800435ec:  sub t2,t2,t5
800435f0:  sra t3,t0,0xf
800435f4:  sra t4,t1,0xf
800435f8:  sra t5,t2,0xf
800435fc:  ldsv_ t3,t4,t5
80043608:  andi t0,t0,0x7fff
8004360c:  andi t1,t1,0x7fff
80043610:  RTIR_SF0
80043614:  andi t2,t2,0x7fff
80043618:  read_mt t3,t4,t5
80043624:  ldsv_ t0,t1,t2
80043630:  sll t3,t3,0x3
80043634:  sll t4,t4,0x3
80043638:  RTIR
8004363c:  sll t5,t5,0x3
80043640:  read_mt t0,t1,t2
8004364c:  addu t0,t0,t3
80043650:  addu t1,t1,t4
80043654:  addu t2,t2,t5
80043658:  sw t0,0x0(a2)
8004365c:  sw t1,0x4(a2)
80043660:  sw t2,0x8(a2)
80043664:  jr t7
80043668:  _move v0,a2
