# addr: 0x80043974  name: FUN_80043974
80043974:  lw t0,0x0(a0)
80043978:  lw t1,0x4(a0)
8004397c:  lw t2,0x8(a0)
80043980:  lw t3,0xc(a0)
80043984:  lw t4,0x10(a0)
80043988:  gte_ldR11R12 t0
8004398c:  gte_ldR13R21 t1
80043990:  gte_ldR22R23 t2
80043994:  gte_ldR31R32 t3
80043998:  gte_ldR33 t4
8004399c:  lw t3,0x14(a0)
800439a0:  lw t4,0x18(a0)
800439a4:  lw t5,0x1c(a0)
800439a8:  gte_ldTRX t3
800439ac:  gte_ldTRY t4
800439b0:  jr ra
800439b4:  _gte_ldTRZ t5
