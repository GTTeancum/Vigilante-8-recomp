# addr: 0x8001db24  name: FUN_8001db24
8001db24:  addiu sp,sp,-0x18
8001db28:  sw s0,0x10(sp)
8001db2c:  sw ra,0x14(sp)
8001db30:  jal 0x8001d624
8001db34:  _move s0,a1
8001db38:  move a0,v0
8001db3c:  jal 0x8001d9c0
8001db40:  _move a1,s0
8001db44:  lw ra,0x14(sp)
8001db48:  lw s0,0x10(sp)
8001db4c:  jr ra
8001db50:  _addiu sp,sp,0x18
