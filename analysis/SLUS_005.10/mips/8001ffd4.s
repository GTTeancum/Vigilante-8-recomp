# addr: 0x8001ffd4  name: FUN_8001ffd4
8001ffd4:  addiu sp,sp,-0x18
8001ffd8:  sw ra,0x10(sp)
8001ffdc:  jal 0x8001ff58
8001ffe0:  _clear a2
8001ffe4:  beq v0,zero,0x8001fff0
8001ffe8:  _clear v1
8001ffec:  lw v1,0x8(v0)
8001fff0:  lw ra,0x10(sp)
8001fff4:  move v0,v1
8001fff8:  jr ra
8001fffc:  _addiu sp,sp,0x18
