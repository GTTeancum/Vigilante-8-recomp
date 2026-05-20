# addr: 0x8004d344  name: SetRotMatrix
8004d344:  lw t0,0x0(a0)
8004d348:  lw t1,0x4(a0)
8004d34c:  lw t2,0x8(a0)
8004d350:  lw t3,0xc(a0)
8004d354:  lw t4,0x10(a0)
8004d358:  gte_ldR11R12 t0
8004d35c:  gte_ldR13R21 t1
8004d360:  gte_ldR22R23 t2
8004d364:  gte_ldR31R32 t3
8004d368:  gte_ldR33 t4
8004d36c:  jr ra
8004d370:  _nop
