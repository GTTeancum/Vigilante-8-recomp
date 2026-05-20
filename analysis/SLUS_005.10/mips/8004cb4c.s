# addr: 0x8004cb4c  name: LoadAverageShort0
8004cb4c:  lw t0,0x0(a0)
8004cb50:  lw t2,0x4(a0)
8004cb54:  sra t1,t0,0x10
8004cb58:  andi t0,t0,0xffff
8004cb5c:  andi t2,t2,0xffff
8004cb60:  gte_ldIR0 a2
8004cb64:  ldsv_ t0,t1,t2
8004cb70:  nop
8004cb74:  GPF0 0x0
8004cb78:  lw t0,0x0(a1)
8004cb7c:  lw t2,0x4(a1)
8004cb80:  sra t1,t0,0x10
8004cb84:  andi t0,t0,0xffff
8004cb88:  andi t2,t2,0xffff
8004cb8c:  gte_stLZCR v0
8004cb90:  gte_ldIR0 a3
8004cb94:  ldsv_ t0,t1,t2
8004cba0:  nop
8004cba4:  GPL0 0x0
8004cba8:  gte_stIR1 t0
8004cbac:  gte_stIR2 t1
8004cbb0:  andi t0,t0,0xffff
8004cbb4:  sll t1,t1,0x10
8004cbb8:  or t0,t0,t1
8004cbbc:  lw t5,0x10(sp)
8004cbc0:  gte_stIR3 t2
8004cbc4:  sw t0,0x0(t5)
8004cbc8:  sw t2,0x4(t5)
8004cbcc:  jr ra
8004cbd0:  _nop
