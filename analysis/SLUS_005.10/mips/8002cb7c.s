# addr: 0x8002cb7c  name: FUN_8002cb7c
8002cb7c:  addiu sp,sp,-0x18
8002cb80:  sw s0,0x10(sp)
8002cb84:  sw ra,0x14(sp)
8002cb88:  jal 0x8001d5a0
8002cb8c:  _move s0,a0
8002cb90:  move a0,v0
8002cb94:  beq a0,zero,0x8002cbd8
8002cb98:  _nop
8002cb9c:  clear a1
8002cba0:  li v1,0x24
8002cba4:  addu v0,a0,v1
8002cba8:  lw v0,0xec(v0)
8002cbac:  nop
8002cbb0:  bne v0,s0,0x8002cbc8
8002cbb4:  _nop
8002cbb8:  jal 0x8002ca94
8002cbbc:  _nop
8002cbc0:  j 0x8002cbd8
8002cbc4:  _nop
8002cbc8:  addiu a1,a1,0x1
8002cbcc:  slti v0,a1,0x3
8002cbd0:  bne v0,zero,0x8002cba4
8002cbd4:  _addiu v1,v1,0x4
8002cbd8:  lw ra,0x14(sp)
8002cbdc:  lw s0,0x10(sp)
8002cbe0:  jr ra
8002cbe4:  _addiu sp,sp,0x18
