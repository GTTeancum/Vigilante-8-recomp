# addr: 0x8001aa38  name: FUN_8001aa38
8001aa38:  addiu sp,sp,-0x18
8001aa3c:  sw s0,0x10(sp)
8001aa40:  move s0,a0
8001aa44:  sw ra,0x14(sp)
8001aa48:  lw a0,0x0(s0)
8001aa4c:  nop
8001aa50:  beq a0,zero,0x8001aa60
8001aa54:  _nop
8001aa58:  jal 0x80045088
8001aa5c:  _nop
8001aa60:  lw a0,0x4(s0)
8001aa64:  nop
8001aa68:  beq a0,zero,0x8001aa78
8001aa6c:  _nop
8001aa70:  jal 0x80045088
8001aa74:  _nop
8001aa78:  lw a0,0x8(s0)
8001aa7c:  nop
8001aa80:  beq a0,zero,0x8001aa90
8001aa84:  _nop
8001aa88:  jal 0x80044394
8001aa8c:  _nop
8001aa90:  jal 0x8001aa0c
8001aa94:  _move a0,s0
8001aa98:  lw ra,0x14(sp)
8001aa9c:  lw s0,0x10(sp)
8001aaa0:  jr ra
8001aaa4:  _addiu sp,sp,0x18
