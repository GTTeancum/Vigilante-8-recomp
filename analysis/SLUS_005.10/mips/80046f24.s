# addr: 0x80046f24  name: S_SVA_OBJ_1A0
80046f24:  lui v0,0x8006
80046f28:  lw v0,-0x122c(v0)
80046f2c:  sll v1,s3,0x1
80046f30:  addu v1,v1,v0
80046f34:  or v0,a1,a0
80046f38:  sh v0,0x0(v1)
80046f3c:  bne s2,zero,0x80046f4c
80046f40:  _andi v0,s1,0x2
80046f44:  beq v0,zero,0x8004701c
80046f48:  _nop
80046f4c:  lhu v0,0xa(s0)
80046f50:  clear a0
80046f54:  bne s2,zero,0x80046f68
80046f58:  _andi a1,v0,0x7fff
80046f5c:  andi v0,s1,0x8
80046f60:  beq v0,zero,0x80046fd4
80046f64:  _nop
80046f68:  lhu v0,0xe(s0)
80046f6c:  nop
80046f70:  addiu v0,v0,-0x1
80046f74:  sll v0,v0,0x10
80046f78:  sra v1,v0,0x10
80046f7c:  sltiu v0,v1,0x7
80046f80:  beq v0,zero,0x80046fd4
80046f84:  _sll v0,v1,0x2
80046f88:  lui at,0x8001
80046f8c:  addu at,at,v0
80046f90:  lw v0,0xc44(at)
80046f94:  nop
80046f98:  jr v0
80046f9c:  _nop
