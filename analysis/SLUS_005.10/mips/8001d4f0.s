# addr: 0x8001d4f0  name: FUN_8001d4f0
8001d4f0:  lw v0,0x38(a0)
8001d4f4:  nop
8001d4f8:  bne v0,zero,0x8001d50c
8001d4fc:  _nop
8001d500:  sw a1,0x38(a0)
8001d504:  jr ra
8001d508:  _sw a0,0x3c(a1)
8001d50c:  move a0,v0
8001d510:  lw v0,0x34(a0)
8001d514:  nop
8001d518:  beq v0,zero,0x8001d538
8001d51c:  _nop
8001d520:  lw a0,0x34(a0)
8001d524:  nop
8001d528:  lw v0,0x34(a0)
8001d52c:  nop
8001d530:  bne v0,zero,0x8001d520
8001d534:  _nop
8001d538:  sw a1,0x34(a0)
8001d53c:  jr ra
8001d540:  _sw a0,0x3c(a1)
