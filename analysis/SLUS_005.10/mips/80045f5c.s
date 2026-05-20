# addr: 0x80045f5c  name: SPU_OBJ_A88
80045f5c:  lui a0,0x8006
80045f60:  lw a0,-0x1218(a0)
80045f64:  lui v1,0xf0ff
80045f68:  lw v0,0x0(a0)
80045f6c:  ori v1,v1,0xffff
80045f70:  and v0,v0,v1
80045f74:  lui v1,0x2000
80045f78:  or v0,v0,v1
80045f7c:  jr ra
80045f80:  _sw v0,0x0(a0)
