# addr: 0x800210a4  name: FUN_800210a4
800210a4:  addiu sp,sp,-0x20
800210a8:  sw s0,0x10(sp)
800210ac:  move s0,a0
800210b0:  sw s1,0x14(sp)
800210b4:  move s1,a1
800210b8:  sw ra,0x18(sp)
800210bc:  lw v0,0x0(s0)
800210c0:  nop
800210c4:  bne v0,zero,0x800210e0
800210c8:  _sltiu v0,v0,0x3
800210cc:  addiu a0,s0,0x4
800210d0:  jal 0x8001ff0c
800210d4:  _move a1,s1
800210d8:  j 0x80021108
800210dc:  _nop
800210e0:  beq v0,zero,0x80021108
800210e4:  _nop
800210e8:  lw a0,0x8(s0)
800210ec:  jal 0x800210a4
800210f0:  _move a1,s1
800210f4:  bne v0,zero,0x80021108
800210f8:  _nop
800210fc:  lw s0,0xc(s0)
80021100:  j 0x800210bc
80021104:  _nop
80021108:  lw ra,0x18(sp)
8002110c:  lw s1,0x14(sp)
80021110:  lw s0,0x10(sp)
80021114:  jr ra
80021118:  _addiu sp,sp,0x20
