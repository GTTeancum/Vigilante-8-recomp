# addr: 0x80045f04  name: _spu_FsetPCR
80045f04:  lui a1,0x8006
80045f08:  lw a1,-0x121c(a1)
80045f0c:  lui v1,0xfff8
80045f10:  lw v0,0x0(a1)
80045f14:  ori v1,v1,0xffff
80045f18:  and v0,v0,v1
80045f1c:  beq a0,zero,0x80045f3c
80045f20:  _sw v0,0x0(a1)
80045f24:  lui v0,0x8006
80045f28:  lw v0,-0x121c(v0)
80045f2c:  nop
80045f30:  lw v1,0x0(v0)
80045f34:  j 0x80045f50
80045f38:  _lui a0,0x3
80045f3c:  lui v0,0x8006
80045f40:  lw v0,-0x121c(v0)
80045f44:  nop
80045f48:  lw v1,0x0(v0)
80045f4c:  lui a0,0x5
