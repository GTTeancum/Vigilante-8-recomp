# addr: 0x80022e38  name: FUN_80022e38
80022e38:  lw v0,0xc(gp)
80022e3c:  addiu sp,sp,-0x18
80022e40:  move a2,a0
80022e44:  slti v0,v0,0x3d
80022e48:  bne v0,zero,0x80022e58
80022e4c:  _sw ra,0x10(sp)
80022e50:  li v0,0x1
80022e54:  sb v0,0x8(a2)
80022e58:  lw a0,0x10c(a2)
80022e5c:  li v0,0x3c
80022e60:  sh zero,0xa4(a2)
80022e64:  sh v0,0xa6(a2)
80022e68:  lw v0,0x64(a0)
80022e6c:  nop
80022e70:  beq v0,zero,0x80022e80
80022e74:  _nop
80022e78:  jalr v0
80022e7c:  _li a1,0xb
80022e80:  lw ra,0x10(sp)
80022e84:  nop
80022e88:  jr ra
80022e8c:  _addiu sp,sp,0x18
