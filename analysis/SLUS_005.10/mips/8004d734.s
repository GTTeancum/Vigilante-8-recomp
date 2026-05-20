# addr: 0x8004d734  name: TransposeMatrix
8004d734:  move v0,a1
8004d738:  lw t1,0x0(a0)
8004d73c:  lw t2,0x4(a0)
8004d740:  sw t1,0x4(a1)
8004d744:  sw t2,0x0(a1)
8004d748:  sh t1,0x0(a1)
8004d74c:  lw t3,0x8(a0)
8004d750:  lw t1,0xc(a0)
8004d754:  sw t3,0xc(a1)
8004d758:  sw t1,0x8(a1)
8004d75c:  sh t2,0xc(a1)
8004d760:  sh t3,0x8(a1)
8004d764:  lh t2,0x10(a0)
8004d768:  sh t1,0x4(a1)
8004d76c:  jr ra
8004d770:  _sh t2,0x10(a1)
