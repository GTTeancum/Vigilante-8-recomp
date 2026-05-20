# addr: 0x8001ac08  name: FUN_8001ac08
8001ac08:  addiu sp,sp,-0x18
8001ac0c:  sw s0,0x10(sp)
8001ac10:  move s0,a0
8001ac14:  sw ra,0x14(sp)
8001ac18:  lw a0,0x58(s0)
8001ac1c:  lhu a1,0xa(s0)
8001ac20:  jal 0x8001ab98
8001ac24:  _nop
8001ac28:  lhu v1,0x6cc(gp)
8001ac2c:  sw v0,0x60(s0)
8001ac30:  sh v1,0x46(s0)
8001ac34:  lw ra,0x14(sp)
8001ac38:  lw s0,0x10(sp)
8001ac3c:  jr ra
8001ac40:  _addiu sp,sp,0x18
