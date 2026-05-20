# addr: 0x80052da4  name: _putchar
80052da4:  addiu sp,sp,-0x18
80052da8:  sw s0,0x10(sp)
80052dac:  move s0,a0
80052db0:  sll a0,a0,0x18
80052db4:  sra a0,a0,0x18
80052db8:  li v0,0x9
80052dbc:  beq a0,v0,0x80052de4
80052dc0:  _sw ra,0x14(sp)
80052dc4:  li v0,0xa
80052dc8:  bne a0,v0,0x80052e0c
80052dcc:  _andi v0,s0,0xff
80052dd0:  jal 0x80052da4
80052dd4:  _li a0,0xd
80052dd8:  lui at,0x8006
80052ddc:  j 0x80052e40
80052de0:  _sw zero,0x5204(at)
