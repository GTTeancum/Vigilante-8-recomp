# addr: 0x8002f998  name: FUN_8002f998
8002f998:  lui at,0x1f80
8002f99c:  sw ra,0x3f0(at)
8002f9a0:  sw sp,0x3f4(at)
8002f9a4:  jal 0x8002f9bc
8002f9a8:  _addi sp,at,0x3e0
8002f9ac:  lw ra,0x10(sp)
8002f9b0:  lw sp,0x14(sp)
8002f9b4:  jr ra
8002f9b8:  _nop
