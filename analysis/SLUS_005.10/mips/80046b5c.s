# addr: 0x80046b5c  name: S_SCA_OBJ_2C8
80046b5c:  sh v0,0x1aa(v1)
80046b60:  bne t2,zero,0x80046b70
80046b64:  _andi v0,t1,0x1000
80046b68:  beq v0,zero,0x80046bb4
80046b6c:  _nop
80046b70:  lw v0,0x20(a0)
80046b74:  nop
80046b78:  bne v0,zero,0x80046b98
80046b7c:  _nop
80046b80:  lui v1,0x8006
80046b84:  lw v1,-0x122c(v1)
80046b88:  nop
80046b8c:  lhu v0,0x1aa(v1)
80046b90:  j 0x80046bb0
80046b94:  _andi v0,v0,0xfff7
80046b98:  lui v1,0x8006
80046b9c:  lw v1,-0x122c(v1)
80046ba0:  nop
80046ba4:  lhu v0,0x1aa(v1)
80046ba8:  nop
80046bac:  ori v0,v0,0x8
