# addr: 0x8004c814  name: VectorNormal
8004c814:  lw t0,0x0(a0)
8004c818:  lw t1,0x4(a0)
8004c81c:  lw t2,0x8(a0)
8004c820:  move a3,ra
8004c824:  jal 0x8004c874
8004c828:  _nop
8004c82c:  sw t0,0x0(a1)
8004c830:  sw t1,0x4(a1)
8004c834:  sw t2,0x8(a1)
8004c838:  move ra,a3
8004c83c:  jr ra
8004c840:  _nop
