# addr: 0x8001d5a0  name: FUN_8001d5a0
8001d5a0:  lw v1,0x3c(a0)
8001d5a4:  nop
8001d5a8:  beq v1,zero,0x8001d5d8
8001d5ac:  _nop
8001d5b0:  lw v0,0x38(v1)
8001d5b4:  nop
8001d5b8:  beq v0,a0,0x8001d5d8
8001d5bc:  _nop
8001d5c0:  lw a0,0x3c(a0)
8001d5c4:  nop
8001d5c8:  lw v1,0x3c(a0)
8001d5cc:  nop
8001d5d0:  bne v1,zero,0x8001d5b0
8001d5d4:  _nop
8001d5d8:  jr ra
8001d5dc:  _move v0,v1
