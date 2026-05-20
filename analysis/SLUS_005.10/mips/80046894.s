# addr: 0x80046894  name: SpuSetCommonAttr
80046894:  clear a2
80046898:  lw t1,0x0(a0)
8004689c:  nop
800468a0:  sltiu t2,t1,0x1
800468a4:  bne t2,zero,0x800468c0
800468a8:  _clear t0
800468ac:  andi v0,t1,0x1
800468b0:  beq v0,zero,0x80046968
800468b4:  _andi v0,t1,0x4
800468b8:  beq v0,zero,0x80046924
800468bc:  _nop
800468c0:  lh v1,0x8(a0)
800468c4:  nop
800468c8:  sltiu v0,v1,0x8
800468cc:  beq v0,zero,0x80046924
800468d0:  _sll v0,v1,0x2
800468d4:  lui at,0x8001
800468d8:  addu at,at,v0
800468dc:  lw v0,0xbe4(at)
800468e0:  nop
800468e4:  jr v0
800468e8:  _nop
