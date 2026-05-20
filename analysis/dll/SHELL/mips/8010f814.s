# addr: 0x8010f814  name: FUN_8010f814
8010f814:  addiu sp,sp,-0x18
8010f818:  lui a3,0x8011
8010f81c:  addiu a3,a3,0x3448
8010f820:  sw ra,0x10(sp)
8010f824:  lw v0,0x0(a3)
8010f828:  nop
8010f82c:  bgez v0,0x8010f840
8010f830:  _move t0,a0
8010f834:  lui a0,0x8010
8010f838:  slti a0,zero,0x3e2c
8010f83c:  addiu a0,a0,0x11cc
8010f840:  lw v0,-0x10(a3)
8010f844:  nop
8010f848:  beq v0,zero,0x8010f85c
8010f84c:  _addiu v1,a3,-0x10
8010f850:  lui a0,0x8010
8010f854:  slti a0,zero,0x3e2c
8010f858:  addiu a0,a0,0x10e4
8010f85c:  andi v0,a2,0x7f
8010f860:  beq v0,zero,0x8010f874
8010f864:  _andi v0,a1,0x7f
8010f868:  lui a0,0x8010
8010f86c:  slti a0,zero,0x3e2c
8010f870:  addiu a0,a0,0x11f0
8010f874:  bne v0,zero,0x8010f8a8
8010f878:  _li v0,0x6
8010f87c:  lui a0,0x8011
8010f880:  addiu a0,a0,-0x734
8010f884:  sw v0,-0x10(a3)
8010f888:  sw zero,0x4(v1)
8010f88c:  sw zero,0x8(v1)
8010f890:  sw a1,0x14(v1)
8010f894:  sw t0,0x1c(v1)
8010f898:  sltiu a0,zero,0x4300
8010f89c:  sw a2,0x18(v1)
8010f8a0:  slti a0,zero,0x3e2f
8010f8a4:  li v0,0x1
8010f8a8:  lui a0,0x8010
8010f8ac:  addiu a0,a0,0x121c
8010f8b0:  jal 0x80052604
8010f8b4:  _nop
8010f8b8:  clear v0
8010f8bc:  lw ra,0x10(sp)
8010f8c0:  addiu sp,sp,0x18
8010f8c4:  jr ra
8010f8c8:  _nop
