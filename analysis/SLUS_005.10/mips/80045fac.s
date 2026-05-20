# addr: 0x80045fac  name: _spu_Fw1ts
80045fac:  addiu sp,sp,-0x8
80045fb0:  li v0,0xd
80045fb4:  sw v0,0x4(sp)
80045fb8:  j 0x80045fec
80045fbc:  _sw zero,0x0(sp)
80045fc0:  lw v1,0x4(sp)
80045fc4:  nop
80045fc8:  sll v0,v1,0x1
80045fcc:  addu v0,v0,v1
80045fd0:  sll v0,v0,0x2
80045fd4:  addu v0,v0,v1
80045fd8:  sw v0,0x4(sp)
80045fdc:  lw v0,0x0(sp)
80045fe0:  nop
80045fe4:  addiu v0,v0,0x1
80045fe8:  sw v0,0x0(sp)
