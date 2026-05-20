# addr: 0x80044f64  name: FUN_80044f64
80044f64:  beq a1,zero,0x80044f84
80044f68:  _andi t0,a0,0x3
80044f6c:  beq t0,zero,0x80044f8c
80044f70:  _nop
80044f74:  sb zero,0x0(a0)
80044f78:  addi a1,a1,-0x1
80044f7c:  bne a1,zero,0x80044f68
80044f80:  _addi a0,a0,0x1
80044f84:  jr ra
80044f88:  _nop
80044f8c:  addiu a1,a1,-0x4
80044f90:  bltz a1,0x80044fa8
80044f94:  _nop
80044f98:  sw zero,0x0(a0)
80044f9c:  addi a1,a1,-0x4
80044fa0:  bgez a1,0x80044f98
80044fa4:  _addi a0,a0,0x4
80044fa8:  addiu a1,a1,0x3
80044fac:  bltz a1,0x80044f84
80044fb0:  _add a0,a0,a1
80044fb4:  jr ra
80044fb8:  _swl zero,0x0(a0)
