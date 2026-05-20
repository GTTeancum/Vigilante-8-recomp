# addr: 0x80053f1c  name: PadInfoComb
80053f1c:  lui v0,0x8006
80053f20:  lw v0,0x5270(v0)
80053f24:  addiu sp,sp,-0x20
80053f28:  sw s0,0x10(sp)
80053f2c:  move s0,a1
80053f30:  sw s1,0x14(sp)
80053f34:  sw ra,0x18(sp)
80053f38:  jalr v0
80053f3c:  _move s1,a2
80053f40:  bgez s0,0x80053f54
80053f44:  _move v1,v0
80053f48:  lbu v0,0xea(v1)
80053f4c:  j 0x80053fb0
80053f50:  _nop
80053f54:  lbu v0,0xea(v1)
80053f58:  nop
80053f5c:  slt v0,s0,v0
80053f60:  beq v0,zero,0x80053fac
80053f64:  _sll v0,s0,0x3
80053f68:  lw v1,0x8(v1)
80053f6c:  bgez s1,0x80053f80
80053f70:  _addu v1,v1,v0
80053f74:  lbu v0,0x0(v1)
80053f78:  j 0x80053fb0
80053f7c:  _nop
80053f80:  lbu v0,0x0(v1)
80053f84:  nop
80053f88:  slt v0,s1,v0
80053f8c:  beq v0,zero,0x80053fb0
80053f90:  _clear v0
80053f94:  lw v0,0x4(v1)
80053f98:  nop
80053f9c:  addu v0,v0,s1
80053fa0:  lbu v0,0x0(v0)
80053fa4:  j 0x80053fb0
80053fa8:  _nop
80053fac:  clear v0
