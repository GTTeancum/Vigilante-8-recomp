# addr: 0x80018f7c  name: FUN_80018f7c
80018f7c:  lw v0,0x4(gp)
80018f80:  nop
80018f84:  sll v0,v0,0x2
80018f88:  addu a0,a0,v0
80018f8c:  lw v0,0x0(a0)
80018f90:  lw a0,0x0(a1)
80018f94:  lw v1,0x0(v0)
80018f98:  addiu v0,v0,0x4
80018f9c:  sll v0,v0,0x8
80018fa0:  srl v0,v0,0x8
80018fa4:  sw v0,0x0(a1)
80018fa8:  lbu v0,0x3(v1)
80018fac:  nop
80018fb0:  sll v0,v0,0x18
80018fb4:  or v0,v0,a0
80018fb8:  jr ra
80018fbc:  _sw v0,0x0(v1)
