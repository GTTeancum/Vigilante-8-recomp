# addr: 0x8004cdc4  name: MulRotMatrix0
8004cdc4:  lhu t0,0x0(a0)
8004cdc8:  lw t1,0x4(a0)
8004cdcc:  lw t2,0xc(a0)
8004cdd0:  lui at,0xffff
8004cdd4:  and t1,t1,at
8004cdd8:  or t0,t0,t1
8004cddc:  gte_ldVXY0 t0
8004cde0:  gte_ldVZ0 t2
8004cde4:  nop
8004cde8:  RTV0
8004cdec:  lhu t0,0x2(a0)
8004cdf0:  lw t1,0x8(a0)
8004cdf4:  lh t2,0xe(a0)
8004cdf8:  sll t1,t1,0x10
8004cdfc:  or t0,t0,t1
8004ce00:  gte_stIR1 t3
8004ce04:  gte_stIR2 t4
8004ce08:  gte_stIR3 t5
8004ce0c:  gte_ldVXY0 t0
8004ce10:  gte_ldVZ0 t2
8004ce14:  nop
8004ce18:  RTV0
8004ce1c:  lhu t0,0x4(a0)
8004ce20:  lw t1,0x8(a0)
8004ce24:  lw t2,0x10(a0)
8004ce28:  lui at,0xffff
8004ce2c:  and t1,t1,at
8004ce30:  or t0,t0,t1
8004ce34:  gte_stIR1 t6
8004ce38:  gte_stIR2 t7
8004ce3c:  gte_stIR3 t8
8004ce40:  gte_ldVXY0 t0
8004ce44:  gte_ldVZ0 t2
8004ce48:  nop
8004ce4c:  RTV0
8004ce50:  andi t3,t3,0xffff
8004ce54:  sll t6,t6,0x10
8004ce58:  or t6,t6,t3
8004ce5c:  sw t6,0x0(a1)
8004ce60:  andi t5,t5,0xffff
8004ce64:  sll t8,t8,0x10
8004ce68:  or t8,t8,t5
8004ce6c:  sw t8,0xc(a1)
8004ce70:  gte_stIR1 t0
8004ce74:  gte_stIR2 t1
8004ce78:  andi t0,t0,0xffff
8004ce7c:  sll t4,t4,0x10
8004ce80:  or t0,t0,t4
8004ce84:  sw t0,0x4(a1)
8004ce88:  andi t7,t7,0xffff
8004ce8c:  sll t1,t1,0x10
8004ce90:  or t1,t1,t7
8004ce94:  sw t1,0x8(a1)
8004ce98:  gte_stIR3 0x10(a1)
8004ce9c:  move v0,a1
8004cea0:  jr ra
8004cea4:  _nop
