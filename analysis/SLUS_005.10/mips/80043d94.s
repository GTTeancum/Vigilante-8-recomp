# addr: 0x80043d94  name: FUN_80043d94
80043d94:  addiu sp,sp,-0x20
80043d98:  andi a0,a0,0xff
80043d9c:  li v0,0x1
80043da0:  bne a0,v0,0x80043de8
80043da4:  _sw ra,0x18(sp)
80043da8:  addiu a0,sp,0x10
80043dac:  jal 0x800493ac
80043db0:  _move a1,v0
80043db4:  addiu a0,sp,0x10
80043db8:  jal 0x80049534
80043dbc:  _sb zero,0x13(sp)
80043dc0:  lw v1,0x8e0(gp)
80043dc4:  nop
80043dc8:  slt v1,v1,v0
80043dcc:  beq v1,zero,0x80043de8
80043dd0:  _clear a1
80043dd4:  li a0,0x9
80043dd8:  jal 0x80048fd0
80043ddc:  _move a2,a1
80043de0:  jal 0x80048fbc
80043de4:  _clear a0
80043de8:  lw ra,0x18(sp)
80043dec:  nop
80043df0:  jr ra
80043df4:  _addiu sp,sp,0x20
