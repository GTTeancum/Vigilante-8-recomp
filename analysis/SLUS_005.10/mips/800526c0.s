# addr: 0x800526c0  name: PRNT_OBJ_7C
800526c0:  addiu s7,s7,0x1
800526c4:  lbu v1,0x0(s7)
800526c8:  nop
800526cc:  sltiu v0,v1,0x79
800526d0:  beq v0,zero,0x80052c88
800526d4:  _sll v0,v1,0x2
800526d8:  lui at,0x8001
800526dc:  addu at,at,v0
800526e0:  lw v0,0x13b4(at)
800526e4:  nop
800526e8:  jr v0
800526ec:  _nop
