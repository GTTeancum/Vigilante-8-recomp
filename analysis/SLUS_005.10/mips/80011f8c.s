# addr: 0x80011f8c  name: FUN_80011f8c
80011f8c:  lui v0,0x8006
80011f90:  addiu v0,v0,0x6458
80011f94:  sll v1,a0,0x4
80011f98:  addu v1,v1,a0
80011f9c:  sll v1,v1,0x1
80011fa0:  addu v1,v1,v0
80011fa4:  lbu v0,0x0(v1)
80011fa8:  nop
80011fac:  beq v0,zero,0x80011fbc
80011fb0:  _li v0,0x53
80011fb4:  jr ra
80011fb8:  _clear v0
80011fbc:  lbu v1,0x1(v1)
80011fc0:  nop
80011fc4:  beq v1,v0,0x80012010
80011fc8:  _slti v0,v1,0x54
80011fcc:  beq v0,zero,0x80011fec
80011fd0:  _li v0,0x23
80011fd4:  beq v1,v0,0x80012008
80011fd8:  _li v0,0x41
80011fdc:  beq v1,v0,0x80012000
80011fe0:  _li v0,0x1
80011fe4:  j 0x80012020
80011fe8:  _nop
80011fec:  li v0,0x73
80011ff0:  beq v1,v0,0x80012018
80011ff4:  _li v0,0x1
80011ff8:  j 0x80012020
80011ffc:  _nop
80012000:  jr ra
80012004:  _li v0,0x2
80012008:  jr ra
8001200c:  _li v0,0x3
80012010:  jr ra
80012014:  _li v0,0x4
80012018:  jr ra
8001201c:  _li v0,0x5
80012020:  jr ra
80012024:  _nop
