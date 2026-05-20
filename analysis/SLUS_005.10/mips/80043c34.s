# addr: 0x80043c34  name: FUN_80043c34
80043c34:  addiu sp,sp,-0x20
80043c38:  andi a0,a0,0xff
80043c3c:  li v0,0x1
80043c40:  bne a0,v0,0x80043cd0
80043c44:  _sw ra,0x18(sp)
80043c48:  lbu v0,0x4(a1)
80043c4c:  nop
80043c50:  andi v0,v0,0x80
80043c54:  bne v0,zero,0x80043cd0
80043c58:  _addiu a0,sp,0x10
80043c5c:  lbu v0,0x3(a1)
80043c60:  nop
80043c64:  sb v0,0x10(sp)
80043c68:  lbu v0,0x4(a1)
80043c6c:  sb zero,0x12(sp)
80043c70:  jal 0x80049534
80043c74:  _sb v0,0x11(sp)
80043c78:  lw v1,0x8e0(gp)
80043c7c:  nop
80043c80:  slt v1,v1,v0
80043c84:  beq v1,zero,0x80043cd0
80043c88:  _nop
80043c8c:  lbu v0,0x8df(gp)
80043c90:  nop
80043c94:  beq v0,zero,0x80043cb8
80043c98:  _li a0,0x9
80043c9c:  clear a1
80043ca0:  jal 0x80048fd0
80043ca4:  _move a2,a1
80043ca8:  jal 0x80048fbc
80043cac:  _clear a0
80043cb0:  j 0x80043cd0
80043cb4:  _nop
80043cb8:  li a0,0x3
80043cbc:  lui a1,0x8006
80043cc0:  addiu a1,a1,0x5be3
80043cc4:  addiu a1,a1,-0x3
80043cc8:  jal 0x80048fd0
80043ccc:  _clear a2
80043cd0:  lw ra,0x18(sp)
80043cd4:  nop
80043cd8:  jr ra
80043cdc:  _addiu sp,sp,0x20
