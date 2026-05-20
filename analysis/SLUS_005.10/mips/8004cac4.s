# addr: 0x8004cac4  name: LoadAverageShort12
8004cac4:  lw t0,0x0(a0)
8004cac8:  lw t2,0x4(a0)
8004cacc:  sra t1,t0,0x10
8004cad0:  andi t0,t0,0xffff
8004cad4:  andi t2,t2,0xffff
8004cad8:  gte_ldIR0 a2
8004cadc:  ldsv_ t0,t1,t2
8004cae8:  nop
8004caec:  GPF12
8004caf0:  lw t0,0x0(a1)
8004caf4:  lw t2,0x4(a1)
8004caf8:  sra t1,t0,0x10
8004cafc:  andi t0,t0,0xffff
8004cb00:  andi t2,t2,0xffff
8004cb04:  gte_stLZCR v0
8004cb08:  gte_ldIR0 a3
8004cb0c:  ldsv_ t0,t1,t2
8004cb18:  nop
8004cb1c:  GPL12
8004cb20:  gte_stIR1 t0
8004cb24:  gte_stIR2 t1
8004cb28:  andi t0,t0,0xffff
8004cb2c:  sll t1,t1,0x10
8004cb30:  or t0,t0,t1
8004cb34:  lw t5,0x10(sp)
8004cb38:  gte_stIR3 t2
8004cb3c:  sw t0,0x0(t5)
8004cb40:  sw t2,0x4(t5)
8004cb44:  jr ra
8004cb48:  _nop
