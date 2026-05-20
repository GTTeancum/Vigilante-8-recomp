# addr: 0x801010c4  name: FUN_801010c4
801010c4:  addiu sp,sp,-0x18
801010c8:  li v0,0x3
801010cc:  beq a1,v0,0x801010dc
801010d0:  _sw ra,0x10(sp)
801010d4:  slti a0,zero,0x439
801010d8:  clear v0
801010dc:  jal 0x8002239c
801010e0:  _move a1,a2
801010e4:  lw ra,0x10(sp)
801010e8:  nop
801010ec:  jr ra
801010f0:  _addiu sp,sp,0x18
