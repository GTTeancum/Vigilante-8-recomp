# addr: 0x8002239c  name: FUN_8002239c
8002239c:  addiu sp,sp,-0x18
800223a0:  sw ra,0x10(sp)
800223a4:  lw a1,0x0(a1)
800223a8:  nop
800223ac:  lbu v1,0x4(a1)
800223b0:  li v0,0x7
800223b4:  bne v1,v0,0x800223cc
800223b8:  _clear a2
800223bc:  lhu a1,0xc(a1)
800223c0:  jal 0x80022320
800223c4:  _nop
800223c8:  sltu a2,zero,v0
800223cc:  lw ra,0x10(sp)
800223d0:  move v0,a2
800223d4:  jr ra
800223d8:  _addiu sp,sp,0x18
