# addr: 0x8004d2b4  name: ApplyMatrixSV
8004d2b4:  lw t0,0x0(a0)
8004d2b8:  lw t1,0x4(a0)
8004d2bc:  lw t2,0x8(a0)
8004d2c0:  lw t3,0xc(a0)
8004d2c4:  lw t4,0x10(a0)
8004d2c8:  gte_ldR11R12 t0
8004d2cc:  gte_ldR13R21 t1
8004d2d0:  gte_ldR22R23 t2
8004d2d4:  gte_ldR31R32 t3
8004d2d8:  gte_ldR33 t4
8004d2dc:  ldv0 a1
8004d2e4:  nop
8004d2e8:  RTV0
8004d2ec:  gte_stIR1 t0
8004d2f0:  gte_stIR2 t1
8004d2f4:  gte_stIR3 t2
8004d2f8:  sh t0,0x0(a2)
8004d2fc:  sh t1,0x2(a2)
8004d300:  sh t2,0x4(a2)
8004d304:  move v0,a2
8004d308:  jr ra
8004d30c:  _nop
