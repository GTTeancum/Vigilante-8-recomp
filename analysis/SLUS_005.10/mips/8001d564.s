# addr: 0x8001d564  name: FUN_8001d564
8001d564:  lw a1,0x3c(a0)
8001d568:  nop
8001d56c:  lw v0,0x38(a1)
8001d570:  lw v1,0x34(a0)
8001d574:  bne v0,a0,0x8001d584
8001d578:  _nop
8001d57c:  j 0x8001d588
8001d580:  _sw v1,0x38(a1)
8001d584:  sw v1,0x34(a1)
8001d588:  beq v1,zero,0x8001d594
8001d58c:  _move v0,a0
8001d590:  sw a1,0x3c(v1)
8001d594:  sw zero,0x34(a0)
8001d598:  jr ra
8001d59c:  _sw zero,0x3c(a0)
