# addr: 0x80048148  name: StopCallback
80048148:  lui v0,0x8006
8004814c:  lw v0,-0x8c(v0)
80048150:  addiu sp,sp,-0x18
80048154:  sw ra,0x10(sp)
80048158:  lw v0,0x10(v0)
8004815c:  nop
80048160:  jalr v0
80048164:  _nop
80048168:  lw ra,0x10(sp)
8004816c:  addiu sp,sp,0x18
80048170:  jr ra
80048174:  _nop
