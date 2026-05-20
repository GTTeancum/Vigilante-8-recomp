# addr: 0x80052de4  name: PUTCHAR_OBJ_40
80052de4:  jal 0x80052da4
80052de8:  _li a0,0x20
80052dec:  lui v0,0x8006
80052df0:  lw v0,0x5204(v0)
80052df4:  nop
80052df8:  andi v0,v0,0x7
80052dfc:  beq v0,zero,0x80052e90
80052e00:  _nop
80052e04:  j 0x80052de4
80052e08:  _nop
