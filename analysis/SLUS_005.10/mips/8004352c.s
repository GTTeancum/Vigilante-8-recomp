# addr: 0x8004352c  name: FUN_8004352c
8004352c:  move t7,ra
80043530:  jal 0x8004366c
80043534:  _nop
80043538:  lw t0,0x0(a1)
8004353c:  lw t1,0x4(a1)
80043540:  lw t2,0x8(a1)
80043544:  sra t3,t0,0xf
80043548:  sra t4,t1,0xf
8004354c:  sra t5,t2,0xf
80043550:  ldsv_ t3,t4,t5
8004355c:  andi t0,t0,0x7fff
80043560:  andi t1,t1,0x7fff
80043564:  RTIR_SF0
80043568:  andi t2,t2,0x7fff
8004356c:  read_mt t3,t4,t5
80043578:  ldsv_ t0,t1,t2
80043584:  sll t3,t3,0x3
80043588:  sll t4,t4,0x3
8004358c:  RTIR
80043590:  sll t5,t5,0x3
80043594:  read_mt t0,t1,t2
800435a0:  addu t0,t0,t3
800435a4:  addu t1,t1,t4
800435a8:  addu t2,t2,t5
800435ac:  sw t0,0x0(a2)
800435b0:  sw t1,0x4(a2)
800435b4:  sw t2,0x8(a2)
800435b8:  jr t7
800435bc:  _move v0,a2
