# addr: 0x8004cbd4  name: LoadAverageByte
8004cbd4:  lbu t0,0x0(a0)
8004cbd8:  lbu t1,0x1(a0)
8004cbdc:  gte_ldIR0 a2
8004cbe0:  gte_ldIR1 t0
8004cbe4:  gte_ldIR2 t1
8004cbe8:  nop
8004cbec:  GPF0 0x0
8004cbf0:  lbu t0,0x0(a1)
8004cbf4:  lbu t1,0x1(a1)
8004cbf8:  gte_stLZCR v0
8004cbfc:  gte_ldIR0 a3
8004cc00:  gte_ldIR1 t0
8004cc04:  gte_ldIR2 t1
8004cc08:  li t3,0xc
8004cc0c:  GPL0 0x0
8004cc10:  lw t5,0x10(sp)
8004cc14:  gte_stMAC1 t0
8004cc18:  gte_stMAC2 t1
8004cc1c:  srav t0,t0,t3
8004cc20:  srav t1,t1,t3
8004cc24:  sb t0,0x0(t5)
8004cc28:  sb t1,0x1(t5)
8004cc2c:  jr ra
8004cc30:  _nop
