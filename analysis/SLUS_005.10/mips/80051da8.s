# addr: 0x80051da8  name: LoadImage2
80051da8:  addiu sp,sp,-0x20
80051dac:  sw s0,0x10(sp)
80051db0:  move s0,a0
80051db4:  sw s1,0x14(sp)
80051db8:  move s1,a1
80051dbc:  lui a0,0x8001
80051dc0:  addiu a0,a0,0x1374
80051dc4:  sw ra,0x18(sp)
80051dc8:  jal 0x8004f5e8
80051dcc:  _move a1,s0
80051dd0:  jal 0x80047e44
80051dd4:  _li a0,-0x1
80051dd8:  lui v1,0x8006
80051ddc:  lw v1,0x5138(v1)
80051de0:  addiu v0,v0,0xf0
80051de4:  lui at,0x8006
80051de8:  sw v0,0x5160(at)
80051dec:  lui at,0x8006
80051df0:  sw zero,0x5164(at)
80051df4:  lw v0,0x0(v1)
80051df8:  j 0x80051e24
80051dfc:  _lui v1,0x100
80051e00:  jal 0x80051bc4
80051e04:  _nop
80051e08:  bne v0,zero,0x80051e80
80051e0c:  _li v0,-0x1
80051e10:  lui v0,0x8006
80051e14:  lw v0,0x5138(v0)
80051e18:  nop
80051e1c:  lw v0,0x0(v0)
80051e20:  lui v1,0x100
