# addr: 0x80043aec  name: FUN_80043aec
80043aec:  addiu sp,sp,-0x18
80043af0:  sw ra,0x10(sp)
80043af4:  jal 0x8004b574
80043af8:  _clear a0
80043afc:  jal 0x8004b690
80043b00:  _nop
80043b04:  move v1,v0
80043b08:  li a0,0x1
80043b0c:  beq v1,a0,0x80043b5c
80043b10:  _slti v0,v1,0x2
80043b14:  beq v0,zero,0x80043b2c
80043b18:  _li v0,0x2
80043b1c:  beq v1,zero,0x80043b4c
80043b20:  _nop
80043b24:  j 0x80043b84
80043b28:  _nop
80043b2c:  beq v1,v0,0x80043b6c
80043b30:  _li v0,0x10
80043b34:  bne v1,v0,0x80043b84
80043b38:  _nop
80043b3c:  sw zero,0x5ac(gp)
80043b40:  sb zero,0x8f8(gp)
80043b44:  j 0x80043b84
80043b48:  _nop
80043b4c:  sw a0,0x5ac(gp)
80043b50:  sb zero,0x8f8(gp)
80043b54:  j 0x80043b84
80043b58:  _nop
80043b5c:  li v0,0x2
80043b60:  sw v0,0x5ac(gp)
80043b64:  j 0x80043b84
80043b68:  _nop
80043b6c:  jal 0x80015610
80043b70:  _nop
80043b74:  beq v0,zero,0x80043b80
80043b78:  _li v1,0x3
80043b7c:  li v1,0x4
80043b80:  sw v1,0x5ac(gp)
80043b84:  lw v0,0x5ac(gp)
80043b88:  nop
80043b8c:  sltiu v0,v0,0x2
80043b90:  bne v0,zero,0x80043ba0
80043b94:  _nop
80043b98:  jal 0x80043a74
80043b9c:  _nop
80043ba0:  lw v0,0x5ac(gp)
80043ba4:  lw ra,0x10(sp)
80043ba8:  nop
80043bac:  jr ra
80043bb0:  _addiu sp,sp,0x18
