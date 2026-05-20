# addr: 0x8004d678  name: NormalColorCol3
8004d678:  ldv3 a0,a1,a2
8004d690:  ldrgb a3
8004d694:  nop
8004d698:  NCCT
8004d69c:  lw t0,0x10(sp)
8004d6a0:  lw t1,0x14(sp)
8004d6a4:  lw t2,0x18(sp)
8004d6a8:  strgb3 t0,t1,t2
8004d6b4:  jr ra
8004d6b8:  _nop
