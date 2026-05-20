# addr: 0x80043248  name: FUN_80043248
80043248:  lw t0,0x0(a0)
8004324c:  lw t1,0x4(a0)
80043250:  lw t2,0x8(a0)
80043254:  sra t3,t0,0xf
80043258:  sra t4,t1,0xf
8004325c:  sra t5,t2,0xf
80043260:  ldsv_ t3,t4,t5
8004326c:  andi t0,t0,0x7fff
80043270:  andi t1,t1,0x7fff
80043274:  RTIR_SF0
80043278:  andi t2,t2,0x7fff
8004327c:  read_mt t3,t4,t5
80043288:  ldsv_ t0,t1,t2
80043294:  sll t3,t3,0x3
80043298:  sll t4,t4,0x3
8004329c:  RTIR
800432a0:  sll t5,t5,0x3
800432a4:  read_mt t0,t1,t2
800432b0:  addu t0,t0,t3
800432b4:  addu t1,t1,t4
800432b8:  addu t2,t2,t5
800432bc:  sw t0,0x0(a1)
800432c0:  sw t1,0x4(a1)
800432c4:  sw t2,0x8(a1)
800432c8:  jr ra
800432cc:  _move v0,a1
