# addr: 0x800126c8  name: FUN_800126c8
800126c8:  lw a0,0x634(gp)
800126cc:  addiu sp,sp,-0x18
800126d0:  beq a0,zero,0x800126e0
800126d4:  _sw ra,0x10(sp)
800126d8:  jal 0x80045088
800126dc:  _nop
800126e0:  lw ra,0x10(sp)
800126e4:  sw zero,0x634(gp)
800126e8:  jr ra
800126ec:  _addiu sp,sp,0x18
