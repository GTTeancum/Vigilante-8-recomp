# addr: 0x8004c844  name: VectorNormalSS
8004c844:  lh t0,0x0(a0)
8004c848:  lh t1,0x2(a0)
8004c84c:  lh t2,0x4(a0)
8004c850:  move a3,ra
8004c854:  jal 0x8004c874
8004c858:  _nop
8004c85c:  sh t0,0x0(a1)
8004c860:  sh t1,0x2(a1)
8004c864:  sh t2,0x4(a1)
8004c868:  move ra,a3
8004c86c:  jr ra
8004c870:  _nop
