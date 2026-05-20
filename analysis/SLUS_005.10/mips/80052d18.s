# addr: 0x80052d18  name: tolower
80052d18:  move v1,a0
80052d1c:  andi v0,v1,0xff
80052d20:  lui at,0x8006
80052d24:  addu at,at,v0
80052d28:  lbu v0,0x5175(at)
80052d2c:  nop
80052d30:  andi v0,v0,0x1
80052d34:  beq v0,zero,0x80052d40
80052d38:  _nop
80052d3c:  addiu v1,a0,0x20
80052d40:  sll v0,v1,0x18
80052d44:  jr ra
80052d48:  _sra v0,v0,0x18
