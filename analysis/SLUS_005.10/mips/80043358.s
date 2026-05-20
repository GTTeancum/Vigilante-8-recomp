# addr: 0x80043358  name: FUN_80043358
80043358:  lw t0,0x0(a0)
8004335c:  lw t1,0x4(a0)
80043360:  lw t2,0x8(a0)
80043364:  lw t3,0xc(a0)
80043368:  lw t4,0x10(a0)
8004336c:  gte_ldR11R12 t0
80043370:  gte_ldR13R21 t1
80043374:  gte_ldR22R23 t2
80043378:  gte_ldR31R32 t3
8004337c:  gte_ldR33 t4
80043380:  lw t0,0x0(a1)
80043384:  lw t1,0x4(a1)
80043388:  lw t2,0x8(a1)
8004338c:  sra t3,t0,0xf
80043390:  sra t4,t1,0xf
80043394:  sra t5,t2,0xf
80043398:  ldsv_ t3,t4,t5
800433a4:  andi t0,t0,0x7fff
800433a8:  andi t1,t1,0x7fff
800433ac:  RTIR_SF0
800433b0:  andi t2,t2,0x7fff
800433b4:  read_mt t3,t4,t5
800433c0:  ldsv_ t0,t1,t2
800433cc:  sll t3,t3,0x3
800433d0:  sll t4,t4,0x3
800433d4:  RTIR
800433d8:  sll t5,t5,0x3
800433dc:  read_mt t0,t1,t2
800433e8:  addu t0,t0,t3
800433ec:  addu t1,t1,t4
800433f0:  addu t2,t2,t5
800433f4:  sw t0,0x0(a2)
800433f8:  sw t1,0x4(a2)
800433fc:  sw t2,0x8(a2)
80043400:  jr ra
80043404:  _move v0,a2
