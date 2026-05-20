# addr: 0x8004b6ec  name: TYPE_OBJ_178
8004b6ec:  jal 0x80048f88
8004b6f0:  _addiu a1,sp,0x818
8004b6f4:  move v1,v0
8004b6f8:  beq v1,s1,0x8004b720
8004b6fc:  _addiu s0,s0,0x1
8004b700:  slti v0,s0,0xa
8004b704:  beq v0,zero,0x8004b720
8004b708:  _li a0,0x1b
8004b70c:  addiu a1,sp,0x10
8004b710:  jal 0x80048fd0
8004b714:  _clear a2
8004b718:  j 0x8004b6ec
8004b71c:  _clear a0
8004b720:  li v0,0x1
8004b724:  beq v1,v0,0x8004b788
8004b728:  _li a0,0x9
8004b72c:  lbu v1,0x818(sp)
8004b730:  nop
8004b734:  andi v0,v1,0x10
8004b738:  bne v0,zero,0x8004b7c4
8004b73c:  _li v0,0x10
8004b740:  andi v0,v1,0x1
8004b744:  beq v0,zero,0x8004b774
8004b748:  _nop
8004b74c:  lbu v0,0x819(sp)
8004b750:  nop
8004b754:  andi v0,v0,0x40
8004b758:  beq v0,zero,0x8004b774
8004b75c:  _nop
8004b760:  lui a0,0x8001
8004b764:  jal 0x80052604
8004b768:  _addiu a0,a0,0x1124
8004b76c:  j 0x8004b7c4
8004b770:  _li v0,0x1
8004b774:  lbu v0,0x818(sp)
8004b778:  nop
8004b77c:  andi v0,v0,0x2
8004b780:  j 0x8004b7c4
8004b784:  _sltu v0,zero,v0
8004b788:  clear a1
8004b78c:  jal 0x80048fd0
8004b790:  _clear a2
8004b794:  addiu a0,sp,0x18
8004b798:  jal 0x800493ac
8004b79c:  _li a1,0x200
8004b7a0:  addiu a0,sp,0x19
8004b7a4:  lui a1,0x8001
8004b7a8:  addiu a1,a1,0x1134
8004b7ac:  jal 0x80052584
8004b7b0:  _li a2,0x5
8004b7b4:  move v1,v0
8004b7b8:  bne v1,zero,0x8004b7c4
8004b7bc:  _li v0,0x1
8004b7c0:  li v0,0x2
