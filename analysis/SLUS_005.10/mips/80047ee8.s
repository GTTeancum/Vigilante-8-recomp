# addr: 0x80047ee8  name: VSYNC_OBJ_A4
80047ee8:  blez a0,0x80047ef4
80047eec:  _clear a1
80047ef0:  addiu a1,a0,-0x1
80047ef4:  jal 0x80047fbc
80047ef8:  _move a0,v0
80047efc:  lui v0,0x8006
80047f00:  lw v0,-0x1184(v0)
80047f04:  nop
80047f08:  lw s0,0x0(v0)
80047f0c:  lui a0,0x8006
80047f10:  lw a0,-0x4c(a0)
80047f14:  li a1,0x1
80047f18:  jal 0x80047fbc
80047f1c:  _addiu a0,a0,0x1
80047f20:  lui v0,0x40
80047f24:  and v0,s0,v0
80047f28:  beq v0,zero,0x80047f68
80047f2c:  _nop
80047f30:  lui v1,0x8006
80047f34:  lw v1,-0x1184(v1)
80047f38:  nop
80047f3c:  lw v0,0x0(v1)
80047f40:  nop
80047f44:  xor v0,s0,v0
80047f48:  bltz v0,0x80047f68
80047f4c:  _lui a0,0x8000
80047f50:  lw v0,0x0(v1)
80047f54:  nop
80047f58:  xor v0,s0,v0
80047f5c:  and v0,v0,a0
80047f60:  beq v0,zero,0x80047f50
80047f64:  _nop
80047f68:  lui v0,0x8006
80047f6c:  lw v0,-0x4c(v0)
80047f70:  lui a0,0x8006
80047f74:  lw a0,-0x1180(a0)
80047f78:  lui at,0x8006
80047f7c:  sw v0,-0x1178(at)
80047f80:  lw v0,0x0(a0)
80047f84:  lui at,0x8006
80047f88:  sw v0,-0x117c(at)
80047f8c:  lui v1,0x8006
80047f90:  lw v1,-0x117c(v1)
80047f94:  lw v0,0x0(a0)
80047f98:  nop
80047f9c:  bne v1,v0,0x80047f80
80047fa0:  _nop
80047fa4:  move v0,s1
