# addr: 0x8002179c  name: FUN_8002179c
8002179c:  addiu sp,sp,-0x18
800217a0:  sw s0,0x10(sp)
800217a4:  move s0,a0
800217a8:  lui a0,0x8006
800217ac:  addiu a0,a0,0x5a18
800217b0:  sw ra,0x14(sp)
800217b4:  jal 0x8001ff0c
800217b8:  _move a1,s0
800217bc:  bne v0,zero,0x800217d0
800217c0:  _nop
800217c4:  lw a0,0x6fc(gp)
800217c8:  jal 0x800210a4
800217cc:  _move a1,s0
800217d0:  lw ra,0x14(sp)
800217d4:  lw s0,0x10(sp)
800217d8:  jr ra
800217dc:  _addiu sp,sp,0x18
