# addr: 0x80052ce4  name: toupper
80052ce4:  move v1,a0
80052ce8:  andi v0,v1,0xff
80052cec:  lui at,0x8006
80052cf0:  addu at,at,v0
80052cf4:  lbu v0,0x5175(at)
80052cf8:  nop
80052cfc:  andi v0,v0,0x2
80052d00:  beq v0,zero,0x80052d0c
80052d04:  _nop
80052d08:  addiu v1,a0,-0x20
80052d0c:  sll v0,v1,0x18
80052d10:  jr ra
80052d14:  _sra v0,v0,0x18
