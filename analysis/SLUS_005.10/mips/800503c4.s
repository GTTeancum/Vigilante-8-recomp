# addr: 0x800503c4  name: SetDrawOffset
800503c4:  addiu sp,sp,-0x18
800503c8:  sw s0,0x10(sp)
800503cc:  move s0,a0
800503d0:  li v0,0x2
800503d4:  sw ra,0x14(sp)
800503d8:  sb v0,0x3(s0)
800503dc:  lh a0,0x0(a1)
800503e0:  lh a1,0x2(a1)
800503e4:  jal 0x80050a60
800503e8:  _nop
800503ec:  sw v0,0x4(s0)
800503f0:  sw zero,0x8(s0)
800503f4:  lw ra,0x14(sp)
800503f8:  lw s0,0x10(sp)
800503fc:  jr ra
80050400:  _addiu sp,sp,0x18
