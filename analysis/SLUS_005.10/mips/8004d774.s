# addr: 0x8004d774  name: RotMatrixX
8004d774:  move t7,a0
8004d778:  move v0,a1
8004d77c:  bgez t7,0x8004d7b8
8004d780:  _andi t9,t7,0xfff
8004d784:  subu t7,zero,t7
8004d788:  bgez t7,0x8004d790
8004d78c:  _andi t7,t7,0xfff
8004d790:  sll t8,t7,0x2
8004d794:  lui t9,0x8006
8004d798:  addu t9,t9,t8
8004d79c:  lw t9,0x7b4(t9)
8004d7a0:  nop
8004d7a4:  sll t6,t9,0x10
8004d7a8:  sra t6,t6,0x10
8004d7ac:  subu t1,zero,t6
8004d7b0:  j 0x8004d7d8
8004d7b4:  _sra t0,t9,0x10
8004d7b8:  sll t8,t9,0x2
8004d7bc:  lui t9,0x8006
8004d7c0:  addu t9,t9,t8
8004d7c4:  lw t9,0x7b4(t9)
8004d7c8:  nop
8004d7cc:  sll t8,t9,0x10
8004d7d0:  sra t1,t8,0x10
8004d7d4:  sra t0,t9,0x10
