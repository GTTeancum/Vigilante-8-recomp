# addr: 0x80020000  name: FUN_80020000
80020000:  addiu sp,sp,-0x18
80020004:  sw ra,0x10(sp)
80020008:  jal 0x8001ff58
8002000c:  _nop
80020010:  beq v0,zero,0x8002001c
80020014:  _clear v1
80020018:  lw v1,0x8(v0)
8002001c:  lw ra,0x10(sp)
80020020:  move v0,v1
80020024:  jr ra
80020028:  _addiu sp,sp,0x18
