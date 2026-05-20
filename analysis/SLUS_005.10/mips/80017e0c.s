# addr: 0x80017e0c  name: FUN_80017e0c
80017e0c:  lw a0,0x6c4(gp)
80017e10:  addiu sp,sp,-0x18
80017e14:  beq a0,zero,0x80017e2c
80017e18:  _sw ra,0x10(sp)
80017e1c:  sw zero,0x6c8(gp)
80017e20:  jal 0x80017db4
80017e24:  _nop
80017e28:  sw zero,0x6c4(gp)
80017e2c:  lw ra,0x10(sp)
80017e30:  nop
80017e34:  jr ra
80017e38:  _addiu sp,sp,0x18
