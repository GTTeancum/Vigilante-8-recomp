# addr: 0x8010e61c  name: FUN_8010e61c
8010e61c:  addiu sp,sp,-0x20
8010e620:  lui v1,0x8011
8010e624:  lw v1,0x24e8(v1)
8010e628:  lui v0,0x10
8010e62c:  sw ra,0x18(sp)
8010e630:  sw v0,0x10(sp)
8010e634:  lw v0,0x0(v1)
8010e638:  lui v1,0x2000
8010e63c:  and v0,v0,v1
8010e640:  beq v0,zero,0x8010e6a0
8010e644:  _clear v0
8010e648:  li a0,-0x1
8010e64c:  lw v0,0x10(sp)
8010e650:  nop
8010e654:  addiu v0,v0,-0x1
8010e658:  sw v0,0x10(sp)
8010e65c:  lw v0,0x10(sp)
8010e660:  nop
8010e664:  bne v0,a0,0x8010e680
8010e668:  _nop
8010e66c:  lui a0,0x8010
8010e670:  sltiu a0,zero,0x39d7
8010e674:  addiu a0,a0,0x10b0
8010e678:  slti a0,zero,0x39a8
8010e67c:  li v0,-0x1
8010e680:  lui v0,0x8011
8010e684:  lw v0,0x24e8(v0)
8010e688:  nop
8010e68c:  lw v0,0x0(v0)
8010e690:  nop
8010e694:  and v0,v0,v1
8010e698:  bne v0,zero,0x8010e64c
8010e69c:  _clear v0
8010e6a0:  lw ra,0x18(sp)
8010e6a4:  addiu sp,sp,0x20
8010e6a8:  jr ra
8010e6ac:  _nop
