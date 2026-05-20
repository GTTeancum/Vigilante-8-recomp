# addr: 0x80011a10  name: FUN_80011a10
80011a10:  lw v0,0x4(gp)
80011a14:  addiu sp,sp,-0x18
80011a18:  li a0,0x1
80011a1c:  sw ra,0x10(sp)
80011a20:  jal 0x800119c0
80011a24:  _subu a0,a0,v0
80011a28:  lw ra,0x10(sp)
80011a2c:  nop
80011a30:  jr ra
80011a34:  _addiu sp,sp,0x18
