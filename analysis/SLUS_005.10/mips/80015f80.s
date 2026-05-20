# addr: 0x80015f80  name: FUN_80015f80
80015f80:  addiu sp,sp,-0x18
80015f84:  sw s0,0x10(sp)
80015f88:  sw ra,0x14(sp)
80015f8c:  jal 0x80015948
80015f90:  _move s0,a0
80015f94:  bne v0,zero,0x80015fa4
80015f98:  _nop
80015f9c:  jal 0x80015368
80015fa0:  _move a0,s0
80015fa4:  lw ra,0x14(sp)
80015fa8:  lw s0,0x10(sp)
80015fac:  jr ra
80015fb0:  _addiu sp,sp,0x18
