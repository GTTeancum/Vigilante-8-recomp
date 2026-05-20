# addr: 0x80045de0  name: _spu_FsetRXX
80045de0:  bne a2,zero,0x80045e00
80045de4:  _sll v0,a0,0x1
80045de8:  lui v1,0x8006
80045dec:  lw v1,-0x122c(v1)
80045df0:  nop
80045df4:  addu v0,v0,v1
80045df8:  j 0x80045e1c
80045dfc:  _sh a1,0x0(v0)
80045e00:  lui a0,0x8006
80045e04:  lw a0,-0x122c(a0)
80045e08:  lui v1,0x8006
80045e0c:  lw v1,-0x1204(v1)
80045e10:  addu v0,v0,a0
80045e14:  srlv v1,a1,v1
80045e18:  sh v1,0x0(v0)
