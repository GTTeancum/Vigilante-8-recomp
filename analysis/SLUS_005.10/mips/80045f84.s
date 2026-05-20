# addr: 0x80045f84  name: SPU_OBJ_AB0
80045f84:  lui a0,0x8006
80045f88:  lw a0,-0x1218(a0)
80045f8c:  lui v1,0xf0ff
80045f90:  lw v0,0x0(a0)
80045f94:  ori v1,v1,0xffff
80045f98:  and v0,v0,v1
80045f9c:  lui v1,0x2200
80045fa0:  or v0,v0,v1
80045fa4:  jr ra
80045fa8:  _sw v0,0x0(a0)
