# addr: 0x80051f80  name: MoveImage2
80051f80:  addiu sp,sp,-0x20
80051f84:  sw s0,0x10(sp)
80051f88:  move s0,a0
80051f8c:  sw s2,0x18(sp)
80051f90:  move s2,a1
80051f94:  sw s1,0x14(sp)
80051f98:  move s1,a2
80051f9c:  lui a0,0x8001
80051fa0:  addiu a0,a0,0x12a4
80051fa4:  sw ra,0x1c(sp)
80051fa8:  jal 0x8004f5e8
80051fac:  _move a1,s0
80051fb0:  jal 0x80047e44
80051fb4:  _li a0,-0x1
80051fb8:  lui v1,0x8006
80051fbc:  lw v1,0x5138(v1)
80051fc0:  addiu v0,v0,0xf0
80051fc4:  lui at,0x8006
80051fc8:  sw v0,0x5160(at)
80051fcc:  lui at,0x8006
80051fd0:  sw zero,0x5164(at)
80051fd4:  lw v0,0x0(v1)
80051fd8:  j 0x80052004
80051fdc:  _lui v1,0x100
80051fe0:  jal 0x80051bc4
80051fe4:  _nop
80051fe8:  bne v0,zero,0x800520ac
80051fec:  _li v0,-0x1
80051ff0:  lui v0,0x8006
80051ff4:  lw v0,0x5138(v0)
80051ff8:  nop
80051ffc:  lw v0,0x0(v0)
80052000:  lui v1,0x100
