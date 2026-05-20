# addr: 0x80042f5c  name: FUN_80042f5c
80042f5c:  addiu sp,sp,-0x18
80042f60:  sw s0,0x10(sp)
80042f64:  move s0,a0
80042f68:  sw ra,0x14(sp)
80042f6c:  lw a0,0x4(s0)
80042f70:  nop
80042f74:  beq a0,zero,0x80042f88
80042f78:  _nop
80042f7c:  jal 0x80045088
80042f80:  _nop
80042f84:  sw zero,0x4(s0)
80042f88:  lw ra,0x14(sp)
80042f8c:  lw s0,0x10(sp)
80042f90:  jr ra
80042f94:  _addiu sp,sp,0x18
