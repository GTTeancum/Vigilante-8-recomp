# addr: 0x80052e40  name: PUTCHAR_OBJ_9C
80052e40:  lui a2,0x8006
80052e44:  lw a2,0x5208(a2)
80052e48:  nop
80052e4c:  slti v0,a2,0x20
80052e50:  bne v0,zero,0x80052e70
80052e54:  _nop
80052e58:  lui a1,0x800a
80052e5c:  addiu a1,a1,0x4c48
80052e60:  jal 0x80053a44
80052e64:  _li a0,0x1
80052e68:  lui at,0x8006
80052e6c:  sw zero,0x5208(at)
80052e70:  lui v1,0x8006
80052e74:  addiu v1,v1,0x5208
80052e78:  lw v0,0x0(v1)
80052e7c:  lui at,0x800a
80052e80:  addu at,at,v0
80052e84:  sb s0,0x4c48(at)
80052e88:  addiu v0,v0,0x1
80052e8c:  sw v0,0x0(v1)
80052e90:  lw ra,0x14(sp)
80052e94:  lw s0,0x10(sp)
80052e98:  jr ra
80052e9c:  _addiu sp,sp,0x18
