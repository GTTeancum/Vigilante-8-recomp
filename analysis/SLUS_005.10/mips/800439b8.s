# addr: 0x800439b8  name: FUN_800439b8
800439b8:  lw t0,0x0(a0)
800439bc:  lw t1,0x4(a0)
800439c0:  lw t2,0x8(a0)
800439c4:  lw t3,0xc(a0)
800439c8:  lw t4,0x10(a0)
800439cc:  gte_ldR11R12 t0
800439d0:  gte_ldR13R21 t1
800439d4:  gte_ldR22R23 t2
800439d8:  gte_ldR31R32 t3
800439dc:  gte_ldR33 t4
800439e0:  li t4,0x1000
800439e4:  gte_ldIR1 t4
800439e8:  gte_ldIR2 a3
800439ec:  sub t0,zero,a2
800439f0:  gte_ldIR3 t0
800439f4:  sub t0,zero,a3
800439f8:  RTIR
800439fc:  gte_stIR1 t1
80043a00:  gte_stIR2 t2
80043a04:  gte_stIR3 t3
80043a08:  ldsv_ t0,t4,a1
80043a14:  nop
80043a18:  RTIR
80043a1c:  sh t1,0x0(a0)
80043a20:  sh t2,0x6(a0)
80043a24:  sh t3,0xc(a0)
80043a28:  sub t0,zero,a1
80043a2c:  gte_stIR1 t1
80043a30:  gte_stIR2 t2
80043a34:  gte_stIR3 t3
80043a38:  ldsv_ a2,t0,t4
80043a44:  nop
80043a48:  RTIR
80043a4c:  sh t1,0x2(a0)
80043a50:  sh t2,0x8(a0)
80043a54:  sh t3,0xe(a0)
80043a58:  gte_stIR1 t1
80043a5c:  gte_stIR2 t2
80043a60:  gte_stIR3 t3
80043a64:  sh t1,0x4(a0)
80043a68:  sh t2,0xa(a0)
80043a6c:  jr ra
80043a70:  _sh t3,0x10(a0)
