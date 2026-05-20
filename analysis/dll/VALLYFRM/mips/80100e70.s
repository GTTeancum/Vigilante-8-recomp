# addr: 0x80100e70  name: FUN_80100e70
80100e70:  addiu sp,sp,-0x18
80100e74:  sw s0,0x10(sp)
80100e78:  move s0,a0
80100e7c:  li v0,0x5
80100e80:  beq a1,v0,0x80100e90
80100e84:  _sw ra,0x14(sp)
80100e88:  slti a0,zero,0x3a9
80100e8c:  clear v0
80100e90:  jal 0x8001d564
80100e94:  _move a0,s0
80100e98:  jal 0x8001af48
80100e9c:  _move a0,s0
80100ea0:  li v0,-0x1
80100ea4:  lw ra,0x14(sp)
80100ea8:  lw s0,0x10(sp)
80100eac:  jr ra
80100eb0:  _addiu sp,sp,0x18
