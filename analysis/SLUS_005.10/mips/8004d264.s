# addr: 0x8004d264  name: ApplyMatrix
8004d264:  lw t0,0x0(a0)
8004d268:  lw t1,0x4(a0)
8004d26c:  lw t2,0x8(a0)
8004d270:  lw t3,0xc(a0)
8004d274:  lw t4,0x10(a0)
8004d278:  gte_ldR11R12 t0
8004d27c:  gte_ldR13R21 t1
8004d280:  gte_ldR22R23 t2
8004d284:  gte_ldR31R32 t3
8004d288:  gte_ldR33 t4
8004d28c:  ldv0 a1
8004d294:  nop
8004d298:  RTV0
8004d29c:  stlvnl a2
8004d2a8:  move v0,a2
8004d2ac:  jr ra
8004d2b0:  _nop
