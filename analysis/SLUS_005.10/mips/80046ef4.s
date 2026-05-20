# addr: 0x80046ef4  name: S_SVA_OBJ_170
80046ef4:  beq a0,zero,0x80046f24
80046ef8:  _nop
80046efc:  lh v1,0x8(s0)
80046f00:  nop
80046f04:  slti v0,v1,0x80
80046f08:  bne v0,zero,0x80046f18
80046f0c:  _nop
80046f10:  j 0x80046f24
80046f14:  _li a1,0x7f
80046f18:  bgez v1,0x80046f24
80046f1c:  _nop
80046f20:  clear a1
