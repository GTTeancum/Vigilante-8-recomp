# addr: 0x800432d0  name: FUN_800432d0
800432d0:  lw t0,0x0(a0)
800432d4:  lw t1,0x4(a0)
800432d8:  lw t2,0x8(a0)
800432dc:  sra t3,t0,0xf
800432e0:  sra t4,t1,0xf
800432e4:  sra t5,t2,0xf
800432e8:  ldsv_ t3,t4,t5
800432f4:  andi t0,t0,0x7fff
800432f8:  andi t1,t1,0x7fff
800432fc:  RTIR_SF0
80043300:  andi t2,t2,0x7fff
80043304:  read_mt t3,t4,t5
80043310:  ldsv_ t0,t1,t2
8004331c:  sll t3,t3,0x3
80043320:  sll t4,t4,0x3
80043324:  RTIRTR
80043328:  sll t5,t5,0x3
8004332c:  read_mt t0,t1,t2
80043338:  addu t0,t0,t3
8004333c:  addu t1,t1,t4
80043340:  addu t2,t2,t5
80043344:  sw t0,0x0(a1)
80043348:  sw t1,0x4(a1)
8004334c:  sw t2,0x8(a1)
80043350:  jr ra
80043354:  _move v0,a1
