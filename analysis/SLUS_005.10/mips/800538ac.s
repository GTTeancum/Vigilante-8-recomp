# addr: 0x800538ac  name: PUTS_OBJ_28
800538ac:  lbu a0,0x0(s0)
800538b0:  nop
800538b4:  sll a0,a0,0x18
800538b8:  bne a0,zero,0x800538a4
800538bc:  _addiu s0,s0,0x1
800538c0:  jal 0x80052ea0
800538c4:  _nop
800538c8:  lw ra,0x14(sp)
800538cc:  lw s0,0x10(sp)
800538d0:  jr ra
800538d4:  _addiu sp,sp,0x18
