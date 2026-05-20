# addr: 0x8010e6b0  name: FUN_8010e6b0
8010e6b0:  addiu sp,sp,-0x20
8010e6b4:  lui v1,0x8011
8010e6b8:  lw v1,0x24c8(v1)
8010e6bc:  lui v0,0x10
8010e6c0:  sw ra,0x18(sp)
8010e6c4:  sw v0,0x10(sp)
8010e6c8:  lw v0,0x0(v1)
8010e6cc:  lui v1,0x100
8010e6d0:  and v0,v0,v1
8010e6d4:  beq v0,zero,0x8010e734
8010e6d8:  _clear v0
8010e6dc:  li a0,-0x1
8010e6e0:  lw v0,0x10(sp)
8010e6e4:  nop
8010e6e8:  addiu v0,v0,-0x1
8010e6ec:  sw v0,0x10(sp)
8010e6f0:  lw v0,0x10(sp)
8010e6f4:  nop
8010e6f8:  bne v0,a0,0x8010e714
8010e6fc:  _nop
8010e700:  lui a0,0x8010
8010e704:  sltiu a0,zero,0x39d7
8010e708:  addiu a0,a0,0x10c0
8010e70c:  slti a0,zero,0x39cd
8010e710:  li v0,-0x1
8010e714:  lui v0,0x8011
8010e718:  lw v0,0x24c8(v0)
8010e71c:  nop
8010e720:  lw v0,0x0(v0)
8010e724:  nop
8010e728:  and v0,v0,v1
8010e72c:  bne v0,zero,0x8010e6e0
8010e730:  _clear v0
8010e734:  lw ra,0x18(sp)
8010e738:  addiu sp,sp,0x20
8010e73c:  jr ra
8010e740:  _nop
