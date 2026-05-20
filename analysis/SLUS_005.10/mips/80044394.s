# addr: 0x80044394  name: FUN_80044394
80044394:  addiu sp,sp,-0x18
80044398:  sw s0,0x10(sp)
8004439c:  move s0,a0
800443a0:  sw ra,0x14(sp)
800443a4:  lhu a0,0x2(s0)
800443a8:  jal 0x80046674
800443ac:  _sll a0,a0,0x3
800443b0:  jal 0x80045088
800443b4:  _move a0,s0
800443b8:  lw ra,0x14(sp)
800443bc:  lw s0,0x10(sp)
800443c0:  jr ra
800443c4:  _addiu sp,sp,0x18
