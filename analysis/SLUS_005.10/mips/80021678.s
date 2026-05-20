# addr: 0x80021678  name: FUN_80021678
80021678:  lui at,0x1f80
8002167c:  sw ra,0x3f0(at)
80021680:  sw sp,0x3f4(at)
80021684:  jal 0x8002169c
80021688:  _addi sp,at,0x3e0
8002168c:  lw ra,0x10(sp)
80021690:  lw sp,0x14(sp)
80021694:  jr ra
80021698:  _nop
