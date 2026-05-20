# addr: 0x80046fd4  name: S_SVA_OBJ_250
80046fd4:  beq a0,zero,0x80047004
80046fd8:  _nop
80046fdc:  lh v1,0xa(s0)
80046fe0:  nop
80046fe4:  slti v0,v1,0x80
80046fe8:  bne v0,zero,0x80046ff8
80046fec:  _nop
80046ff0:  j 0x80047004
80046ff4:  _li a1,0x7f
80046ff8:  bgez v1,0x80047004
80046ffc:  _nop
80047000:  clear a1
