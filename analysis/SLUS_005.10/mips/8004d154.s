# addr: 0x8004d154  name: MulMatrix2
8004d154:  lw t0,0x0(a0)
8004d158:  lw t1,0x4(a0)
8004d15c:  lw t2,0x8(a0)
8004d160:  lw t3,0xc(a0)
8004d164:  lw t4,0x10(a0)
8004d168:  gte_ldR11R12 t0
8004d16c:  gte_ldR13R21 t1
8004d170:  gte_ldR22R23 t2
8004d174:  gte_ldR31R32 t3
8004d178:  gte_ldR33 t4
8004d17c:  lhu t0,0x0(a1)
8004d180:  lw t1,0x4(a1)
8004d184:  lw t2,0xc(a1)
8004d188:  lui at,0xffff
8004d18c:  and t1,t1,at
8004d190:  or t0,t0,t1
8004d194:  gte_ldVXY0 t0
8004d198:  gte_ldVZ0 t2
8004d19c:  nop
8004d1a0:  RTV0
8004d1a4:  lhu t0,0x2(a1)
8004d1a8:  lw t1,0x8(a1)
8004d1ac:  lh t2,0xe(a1)
8004d1b0:  sll t1,t1,0x10
8004d1b4:  or t0,t0,t1
8004d1b8:  gte_stIR1 t3
8004d1bc:  gte_stIR2 t4
8004d1c0:  gte_stIR3 t5
8004d1c4:  gte_ldVXY0 t0
8004d1c8:  gte_ldVZ0 t2
8004d1cc:  nop
8004d1d0:  RTV0
8004d1d4:  lhu t0,0x4(a1)
8004d1d8:  lw t1,0x8(a1)
8004d1dc:  lw t2,0x10(a1)
8004d1e0:  lui at,0xffff
8004d1e4:  and t1,t1,at
8004d1e8:  or t0,t0,t1
8004d1ec:  gte_stIR1 t6
8004d1f0:  gte_stIR2 t7
8004d1f4:  gte_stIR3 t8
8004d1f8:  gte_ldVXY0 t0
8004d1fc:  gte_ldVZ0 t2
8004d200:  nop
8004d204:  RTV0
8004d208:  andi t3,t3,0xffff
8004d20c:  sll t6,t6,0x10
8004d210:  or t6,t6,t3
8004d214:  sw t6,0x0(a1)
8004d218:  andi t5,t5,0xffff
8004d21c:  sll t8,t8,0x10
8004d220:  or t8,t8,t5
8004d224:  sw t8,0xc(a1)
8004d228:  gte_stIR1 t0
8004d22c:  gte_stIR2 t1
8004d230:  andi t0,t0,0xffff
8004d234:  sll t4,t4,0x10
8004d238:  or t0,t0,t4
8004d23c:  sw t0,0x4(a1)
8004d240:  andi t7,t7,0xffff
8004d244:  sll t1,t1,0x10
8004d248:  or t1,t1,t7
8004d24c:  sw t1,0x8(a1)
8004d250:  gte_stIR3 0x10(a1)
8004d254:  move v0,a1
8004d258:  jr ra
8004d25c:  _nop
