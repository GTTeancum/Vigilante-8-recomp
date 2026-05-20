# addr: 0x8004d610  name: NormalColorDpq3
8004d610:  ldv3 a0,a1,a2
8004d628:  ldrgb a3
8004d62c:  gte_ldIR0 0x10(sp)
8004d630:  nop
8004d634:  NCDT
8004d638:  lw t0,0x14(sp)
8004d63c:  lw t1,0x18(sp)
8004d640:  lw t2,0x1c(sp)
8004d644:  strgb3 t0,t1,t2
8004d650:  jr ra
8004d654:  _nop
