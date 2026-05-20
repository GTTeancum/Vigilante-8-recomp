# addr: 0x80052f78  name: PUTCHAR_OBJ_1D4
80052f78:  lui a2,0x8006
80052f7c:  lw a2,0x5208(a2)
80052f80:  nop
80052f84:  slti v0,a2,0x20
80052f88:  bne v0,zero,0x80052fa8
80052f8c:  _nop
80052f90:  lui a1,0x800a
80052f94:  addiu a1,a1,0x4c48
80052f98:  jal 0x80053a44
80052f9c:  _li a0,0x1
80052fa0:  lui at,0x8006
80052fa4:  sw zero,0x5208(at)
80052fa8:  lui v0,0x8006
80052fac:  lw v0,0x5208(v0)
80052fb0:  lui at,0x800a
80052fb4:  addu at,at,v0
80052fb8:  sb s0,0x4c48(at)
80052fbc:  addiu v0,v0,0x1
80052fc0:  lui at,0x8006
80052fc4:  sw v0,0x5208(at)
80052fc8:  lui a2,0x8006
80052fcc:  lw a2,0x5208(a2)
80052fd0:  nop
80052fd4:  blez a2,0x80052ff4
80052fd8:  _nop
80052fdc:  lui a1,0x800a
80052fe0:  addiu a1,a1,0x4c48
80052fe4:  jal 0x80053a44
80052fe8:  _li a0,0x1
80052fec:  lui at,0x8006
80052ff0:  sw zero,0x5208(at)
80052ff4:  lw ra,0x14(sp)
80052ff8:  lw s0,0x10(sp)
80052ffc:  jr ra
80053000:  _addiu sp,sp,0x18
