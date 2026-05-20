# addr: 0x8002479c  name: FUN_8002479c
8002479c:  beq a0,zero,0x800247d4
800247a0:  _nop
800247a4:  lbu v0,0x10(a0)
800247a8:  lw v1,0x8(a0)
800247ac:  sll v0,v0,0x1
800247b0:  addu v1,v1,v0
800247b4:  lhu v0,0x2(v1)
800247b8:  nop
800247bc:  andi v0,v0,0x9fff
800247c0:  sh v0,0x2(v1)
800247c4:  lw a0,0x0(a0)
800247c8:  nop
800247cc:  bne a0,zero,0x800247a4
800247d0:  _nop
800247d4:  jr ra
800247d8:  _nop
