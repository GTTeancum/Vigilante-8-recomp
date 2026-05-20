# addr: 0x8004cc34  name: LoadAverageCol
8004cc34:  lbu t0,0x0(a0)
8004cc38:  lbu t1,0x1(a0)
8004cc3c:  lbu t2,0x2(a0)
8004cc40:  gte_ldIR0 a2
8004cc44:  ldsv_ t0,t1,t2
8004cc50:  nop
8004cc54:  GPF0 0x0
8004cc58:  lbu t0,0x0(a1)
8004cc5c:  lbu t1,0x1(a1)
8004cc60:  lbu t2,0x2(a1)
8004cc64:  gte_stLZCR v0
8004cc68:  gte_ldIR0 a3
8004cc6c:  ldsv_ t0,t1,t2
8004cc78:  li t3,0xc
8004cc7c:  GPL0 0x0
8004cc80:  lw t5,0x10(sp)
8004cc84:  read_mt t0,t1,t2
8004cc90:  srav t0,t0,t3
8004cc94:  srav t1,t1,t3
8004cc98:  srav t2,t2,t3
8004cc9c:  sb t0,0x0(t5)
8004cca0:  sb t1,0x1(t5)
8004cca4:  sb t2,0x2(t5)
8004cca8:  jr ra
8004ccac:  _nop
