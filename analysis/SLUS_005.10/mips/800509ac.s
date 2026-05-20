# addr: 0x800509ac  name: SYS_OBJ_17A8
800509ac:  andi v1,a1,0x3ff
800509b0:  sll v1,v1,0xa
800509b4:  andi v0,a0,0x3ff
800509b8:  lui a0,0xe300
800509bc:  or v0,v0,a0
800509c0:  jr ra
800509c4:  _or v0,v1,v0
