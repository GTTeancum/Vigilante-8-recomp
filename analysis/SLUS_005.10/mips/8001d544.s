# addr: 0x8001d544  name: FUN_8001d544
8001d544:  lw v0,0x38(a0)
8001d548:  sw a1,0x38(a0)
8001d54c:  sw a0,0x3c(a1)
8001d550:  beq v0,zero,0x8001d55c
8001d554:  _sw v0,0x34(a1)
8001d558:  sw a1,0x3c(v0)
8001d55c:  jr ra
8001d560:  _nop
