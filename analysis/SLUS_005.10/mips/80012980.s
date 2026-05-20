# addr: 0x80012980  name: FUN_80012980
80012980:  lw a0,0x608(gp)
80012984:  addiu sp,sp,-0x18
80012988:  beq a0,zero,0x8001299c
8001298c:  _sw ra,0x10(sp)
80012990:  jal 0x80045088
80012994:  _nop
80012998:  sw zero,0x608(gp)
8001299c:  lw ra,0x10(sp)
800129a0:  nop
800129a4:  jr ra
800129a8:  _addiu sp,sp,0x18
