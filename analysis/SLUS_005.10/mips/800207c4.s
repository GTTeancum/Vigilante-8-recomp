# addr: 0x800207c4  name: FUN_800207c4
800207c4:  addiu sp,sp,-0x18
800207c8:  move a1,a0
800207cc:  sw ra,0x10(sp)
800207d0:  lw v0,0x0(a1)
800207d4:  lui a0,0x8006
800207d8:  addiu a0,a0,0x5a80
800207dc:  ori v0,v0,0x4
800207e0:  jal 0x8001fe50
800207e4:  _sw v0,0x0(a1)
800207e8:  lw ra,0x10(sp)
800207ec:  nop
800207f0:  jr ra
800207f4:  _addiu sp,sp,0x18
