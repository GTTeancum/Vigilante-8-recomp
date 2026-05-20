# addr: 0x800532a0  name: SPRINTF_OBJ_29C
800532a0:  sltiu v0,v1,0x2d
800532a4:  beq v0,zero,0x8005378c
800532a8:  _sll v0,v1,0x2
800532ac:  lui at,0x8001
800532b0:  addu at,at,v0
800532b4:  lw v0,0x15cc(at)
800532b8:  nop
800532bc:  jr v0
800532c0:  _nop
