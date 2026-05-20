# addr: 0x800441c8  name: FUN_800441c8
800441c8:  addiu sp,sp,-0x18
800441cc:  beq a0,zero,0x800441e8
800441d0:  _sw ra,0x10(sp)
800441d4:  addiu v0,a0,-0x1
800441d8:  li a1,0x1
800441dc:  clear a0
800441e0:  jal 0x80043ff0
800441e4:  _sllv a1,a1,v0
800441e8:  lw ra,0x10(sp)
800441ec:  nop
800441f0:  jr ra
800441f4:  _addiu sp,sp,0x18
