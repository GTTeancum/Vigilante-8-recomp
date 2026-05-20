# addr: 0x8001ab98  name: FUN_8001ab98
8001ab98:  lw v1,0x4(a0)
8001ab9c:  nop
8001aba0:  beq v1,zero,0x8001abc8
8001aba4:  _clear a0
8001aba8:  andi v0,a1,0xffff
8001abac:  sll v0,v0,0x2
8001abb0:  addu v0,v1,v0
8001abb4:  lw v0,0x4(v0)
8001abb8:  nop
8001abbc:  beq v0,zero,0x8001abc8
8001abc0:  _nop
8001abc4:  addu a0,v1,v0
8001abc8:  jr ra
8001abcc:  _move v0,a0
