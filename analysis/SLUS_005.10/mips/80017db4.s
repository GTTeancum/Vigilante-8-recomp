# addr: 0x80017db4  name: FUN_80017db4
80017db4:  addiu sp,sp,-0x18
80017db8:  sw s0,0x10(sp)
80017dbc:  move s0,a0
80017dc0:  sw ra,0x14(sp)
80017dc4:  lw v0,0x8(s0)
80017dc8:  nop
80017dcc:  addiu v0,v0,-0x2
80017dd0:  sltiu v0,v0,0x2
80017dd4:  beq v0,zero,0x80017df4
80017dd8:  _nop
80017ddc:  lw a0,0x10(s0)
80017de0:  jal 0x80017db4
80017de4:  _nop
80017de8:  lw a0,0x14(s0)
80017dec:  jal 0x80017db4
80017df0:  _nop
80017df4:  jal 0x80045088
80017df8:  _move a0,s0
80017dfc:  lw ra,0x14(sp)
80017e00:  lw s0,0x10(sp)
80017e04:  jr ra
80017e08:  _addiu sp,sp,0x18
