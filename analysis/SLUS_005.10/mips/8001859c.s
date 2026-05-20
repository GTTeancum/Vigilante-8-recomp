# addr: 0x8001859c  name: FUN_8001859c
8001859c:  addiu sp,sp,-0x18
800185a0:  move a1,a0
800185a4:  andi a0,a1,0x3f
800185a8:  andi a1,a1,0xffff
800185ac:  sll a0,a0,0x4
800185b0:  sw ra,0x10(sp)
800185b4:  jal 0x80018530
800185b8:  _srl a1,a1,0x6
800185bc:  lw ra,0x10(sp)
800185c0:  nop
800185c4:  jr ra
800185c8:  _addiu sp,sp,0x18
