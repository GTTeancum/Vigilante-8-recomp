# addr: 0x800466f4  name: SpuRead
800466f4:  addiu sp,sp,-0x18
800466f8:  sw s0,0x10(sp)
800466fc:  move s0,a1
80046700:  lui v0,0x7
80046704:  ori v0,v0,0xeff0
80046708:  sltu v0,v0,s0
8004670c:  beq v0,zero,0x8004671c
80046710:  _sw ra,0x14(sp)
80046714:  lui s0,0x7
80046718:  ori s0,s0,0xeff0
8004671c:  jal 0x80045cf8
80046720:  _move a1,s0
80046724:  lui v0,0x8006
80046728:  lw v0,-0x11f4(v0)
8004672c:  nop
80046730:  bne v0,zero,0x80046740
80046734:  _move v0,s0
80046738:  lui at,0x8006
8004673c:  sw zero,-0x11f8(at)
80046740:  lw ra,0x14(sp)
80046744:  lw s0,0x10(sp)
80046748:  jr ra
8004674c:  _addiu sp,sp,0x18
