# addr: 0x8004d3a4  name: SetColorMatrix
8004d3a4:  lw t0,0x0(a0)
8004d3a8:  lw t1,0x4(a0)
8004d3ac:  lw t2,0x8(a0)
8004d3b0:  lw t3,0xc(a0)
8004d3b4:  lw t4,0x10(a0)
8004d3b8:  gte_ldLR1LR2 t0
8004d3bc:  gte_ldLR3LG1 t1
8004d3c0:  gte_ldLG2LG3 t2
8004d3c4:  gte_ldLB1LB2 t3
8004d3c8:  gte_ldLB3 t4
8004d3cc:  jr ra
8004d3d0:  _nop
