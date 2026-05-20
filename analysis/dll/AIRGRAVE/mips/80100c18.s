# addr: 0x80100c18  name: FUN_80100c18
80100c18:  addiu sp,sp,-0x18
80100c1c:  beq a1,zero,0x80100c38
80100c20:  _sw ra,0x10(sp)
80100c24:  li v0,0x5
80100c28:  beq a1,v0,0x80100c4c
80100c2c:  _clear v0
80100c30:  slti a0,zero,0x318
80100c34:  nop
80100c38:  lw v0,0x28(a0)
80100c3c:  nop
80100c40:  addiu v0,v0,-0x5f5
80100c44:  slti a0,zero,0x317
80100c48:  sw v0,0x28(a0)
80100c4c:  jal 0x800205f8
80100c50:  _nop
80100c54:  slti a0,zero,0x318
80100c58:  li v0,-0x1
80100c5c:  clear v0
80100c60:  lw ra,0x10(sp)
80100c64:  nop
80100c68:  jr ra
80100c6c:  _addiu sp,sp,0x18
