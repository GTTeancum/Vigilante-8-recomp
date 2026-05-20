# addr: 0x80018efc  name: FUN_80018efc
80018efc:  addiu sp,sp,-0x18
80018f00:  sw s0,0x10(sp)
80018f04:  move s0,a0
80018f08:  sw ra,0x14(sp)
80018f0c:  jal 0x800116f4
80018f10:  _li a0,0x8
80018f14:  move a0,s0
80018f18:  move s0,v0
80018f1c:  jal 0x80018e28
80018f20:  _sw a0,0x0(s0)
80018f24:  sw v0,0x4(s0)
80018f28:  lw ra,0x14(sp)
80018f2c:  move v0,s0
80018f30:  lw s0,0x10(sp)
80018f34:  jr ra
80018f38:  _addiu sp,sp,0x18
