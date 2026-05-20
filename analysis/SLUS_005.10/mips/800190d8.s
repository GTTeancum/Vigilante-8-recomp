# addr: 0x800190d8  name: FUN_800190d8
800190d8:  addiu sp,sp,-0x18
800190dc:  sw s0,0x10(sp)
800190e0:  move s0,a0
800190e4:  sw ra,0x14(sp)
800190e8:  lw a0,0x0(s0)
800190ec:  jal 0x80045088
800190f0:  _nop
800190f4:  jal 0x800190a8
800190f8:  _move a0,s0
800190fc:  lw ra,0x14(sp)
80019100:  lw s0,0x10(sp)
80019104:  jr ra
80019108:  _addiu sp,sp,0x18
