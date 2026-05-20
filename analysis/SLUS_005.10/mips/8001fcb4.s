# addr: 0x8001fcb4  name: FUN_8001fcb4
8001fcb4:  addiu sp,sp,-0x20
8001fcb8:  sw s1,0x14(sp)
8001fcbc:  move s1,a0
8001fcc0:  sw s0,0x10(sp)
8001fcc4:  andi s0,a1,0xffff
8001fcc8:  sw ra,0x18(sp)
8001fccc:  jal 0x8001f9cc
8001fcd0:  _move a1,s0
8001fcd4:  bltz v0,0x8001fcf4
8001fcd8:  _nop
8001fcdc:  lw a0,0x38(s1)
8001fce0:  nop
8001fce4:  beq a0,zero,0x8001fcf4
8001fce8:  _nop
8001fcec:  jal 0x8001fc38
8001fcf0:  _move a1,s0
8001fcf4:  lw ra,0x18(sp)
8001fcf8:  lw s1,0x14(sp)
8001fcfc:  lw s0,0x10(sp)
8001fd00:  jr ra
8001fd04:  _addiu sp,sp,0x20
