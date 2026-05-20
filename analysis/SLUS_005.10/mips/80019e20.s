# addr: 0x80019e20  name: FUN_80019e20
80019e20:  addiu sp,sp,-0x80
80019e24:  addiu a0,sp,0x18
80019e28:  clear a1
80019e2c:  move a2,a1
80019e30:  li a3,0x280
80019e34:  li v0,0x1e0
80019e38:  sw ra,0x78(sp)
80019e3c:  jal 0x8004f0e4
80019e40:  _sw v0,0x10(sp)
80019e44:  clear a0
80019e48:  li a1,0x1
80019e4c:  move a2,a0
80019e50:  move a3,a0
80019e54:  move v0,a1
80019e58:  jal 0x80052214
80019e5c:  _sb v0,0x2f(sp)
80019e60:  addiu a0,sp,0x18
80019e64:  jal 0x8004fbe4
80019e68:  _sh v0,0x2c(sp)
80019e6c:  lw ra,0x78(sp)
80019e70:  nop
80019e74:  jr ra
80019e78:  _addiu sp,sp,0x80
