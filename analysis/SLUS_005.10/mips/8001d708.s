# addr: 0x8001d708  name: FUN_8001d708
8001d708:  addiu sp,sp,-0x18
8001d70c:  move a1,a0
8001d710:  addiu a0,a1,0x40
8001d714:  sw ra,0x10(sp)
8001d718:  lw v1,0x48(a1)
8001d71c:  lw a2,0x4c(a1)
8001d720:  lw a3,0x50(a1)
8001d724:  sw v1,0x24(a1)
8001d728:  sw a2,0x28(a1)
8001d72c:  sw a3,0x2c(a1)
8001d730:  jal 0x8004dab4
8001d734:  _addiu a1,a1,0x10
8001d738:  lw ra,0x10(sp)
8001d73c:  nop
8001d740:  jr ra
8001d744:  _addiu sp,sp,0x18
