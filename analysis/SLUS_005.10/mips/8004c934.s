# addr: 0x8004c934  name: MatrixNormal
8004c934:  lh t0,0x0(a0)
8004c938:  lh t1,0x2(a0)
8004c93c:  lh t2,0x4(a0)
8004c940:  lh t3,0x6(a0)
8004c944:  lh t4,0x8(a0)
8004c948:  lh t5,0xa(a0)
8004c94c:  gte_stR11R12 v0
8004c950:  gte_stR22R23 v1
8004c954:  gte_stR33 a2
8004c958:  gte_ldR11R12 t0
8004c95c:  gte_ldR22R23 t1
8004c960:  gte_ldR33 t2
8004c964:  gte_ldIR3 t5
8004c968:  gte_ldIR1 t3
8004c96c:  gte_ldIR2 t4
8004c970:  nop
8004c974:  OP12
8004c978:  read_mt t7,t8,t9
8004c984:  gte_ldR11R12 t3
8004c988:  gte_ldR22R23 t4
8004c98c:  gte_ldR33 t5
8004c990:  nop
8004c994:  OP12
8004c998:  gte_ldVXY0 t3
8004c99c:  gte_ldVZ0 t4
8004c9a0:  gte_ldVXY1 t5
8004c9a4:  read_mt t0,t1,t2
8004c9b0:  gte_ldR11R12 v0
8004c9b4:  gte_ldR22R23 v1
8004c9b8:  gte_ldR33 a2
8004c9bc:  move a3,ra
8004c9c0:  jal 0x8004c874
8004c9c4:  _nop
8004c9c8:  sh t0,0x0(a1)
8004c9cc:  sh t1,0x2(a1)
8004c9d0:  sh t2,0x4(a1)
8004c9d4:  gte_stVXY0 t0
8004c9d8:  gte_stVZ0 t1
8004c9dc:  gte_stVXY1 t2
8004c9e0:  jal 0x8004c874
8004c9e4:  _nop
8004c9e8:  sh t0,0x6(a1)
8004c9ec:  sh t1,0x8(a1)
8004c9f0:  sh t2,0xa(a1)
8004c9f4:  move t0,t7
8004c9f8:  move t1,t8
8004c9fc:  jal 0x8004c874
8004ca00:  _move t2,t9
8004ca04:  sh t0,0xc(a1)
8004ca08:  sh t1,0xe(a1)
8004ca0c:  sh t2,0x10(a1)
8004ca10:  move ra,a3
8004ca14:  jr ra
8004ca18:  _nop
