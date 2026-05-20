# addr: 0x80044efc  name: FUN_80044efc
80044efc:  beq a2,zero,0x80044f20
80044f00:  _move v0,a0
80044f04:  andi t0,a0,0x3
80044f08:  beq t0,zero,0x80044f28
80044f0c:  _andi a1,a1,0xff
80044f10:  sb a1,0x0(a0)
80044f14:  addi a2,a2,-0x1
80044f18:  bne a2,zero,0x80044f04
80044f1c:  _addi a0,a0,0x1
80044f20:  jr ra
80044f24:  _nop
80044f28:  sll t0,a1,0x8
80044f2c:  or a1,a1,t0
80044f30:  sll t0,a1,0x10
80044f34:  addiu a2,a2,-0x4
80044f38:  bltz a2,0x80044f50
80044f3c:  _or a1,a1,t0
80044f40:  sw a1,0x0(a0)
80044f44:  addi a2,a2,-0x4
80044f48:  bgez a2,0x80044f40
80044f4c:  _addi a0,a0,0x4
80044f50:  addiu a2,a2,0x3
80044f54:  bltz a2,0x80044f20
80044f58:  _add a0,a0,a2
80044f5c:  jr ra
80044f60:  _swl a1,0x0(a0)
