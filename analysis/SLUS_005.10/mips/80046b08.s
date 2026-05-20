# addr: 0x80046b08  name: S_SCA_OBJ_274
80046b08:  sh v0,0x1aa(v1)
80046b0c:  bne t2,zero,0x80046b1c
80046b10:  _andi v0,t1,0x200
80046b14:  beq v0,zero,0x80046b60
80046b18:  _nop
80046b1c:  lw v0,0x18(a0)
80046b20:  nop
80046b24:  bne v0,zero,0x80046b44
80046b28:  _nop
80046b2c:  lui v1,0x8006
80046b30:  lw v1,-0x122c(v1)
80046b34:  nop
80046b38:  lhu v0,0x1aa(v1)
80046b3c:  j 0x80046b5c
80046b40:  _andi v0,v0,0xfffe
80046b44:  lui v1,0x8006
80046b48:  lw v1,-0x122c(v1)
80046b4c:  nop
80046b50:  lhu v0,0x1aa(v1)
80046b54:  nop
80046b58:  ori v0,v0,0x1
