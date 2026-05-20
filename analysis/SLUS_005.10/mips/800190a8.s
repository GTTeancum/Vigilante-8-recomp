# addr: 0x800190a8  name: FUN_800190a8
800190a8:  addiu sp,sp,-0x18
800190ac:  sw s0,0x10(sp)
800190b0:  move s0,a0
800190b4:  sw ra,0x14(sp)
800190b8:  jal 0x8001884c
800190bc:  _addiu a0,s0,0x8
800190c0:  jal 0x80045088
800190c4:  _move a0,s0
800190c8:  lw ra,0x14(sp)
800190cc:  lw s0,0x10(sp)
800190d0:  jr ra
800190d4:  _addiu sp,sp,0x18
