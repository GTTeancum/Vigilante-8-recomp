# addr: 0x80045ec8  name: _spu_FgetRXXa
80045ec8:  lui v0,0x8006
80045ecc:  lw v0,-0x122c(v0)
80045ed0:  sll a0,a0,0x1
80045ed4:  addu a0,a0,v0
80045ed8:  li v0,-0x1
80045edc:  lhu a0,0x0(a0)
80045ee0:  beq a1,v0,0x80045ef8
80045ee4:  _nop
80045ee8:  lui v0,0x8006
80045eec:  lw v0,-0x1204(v0)
80045ef0:  j 0x80045efc
80045ef4:  _sllv v0,a0,v0
80045ef8:  move v0,a0
