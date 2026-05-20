# addr: 0x8010edf0  name: FUN_8010edf0
8010edf0:  addiu sp,sp,-0x18
8010edf4:  andi v0,a0,0xff
8010edf8:  sw ra,0x10(sp)
8010edfc:  lui v1,0x8006
8010ee00:  addu v1,v1,v0
8010ee04:  lbu v1,0x5175(v1)
8010ee08:  nop
8010ee0c:  andi v0,v1,0x4
8010ee10:  beq v0,zero,0x8010ee24
8010ee14:  _sll v0,a0,0x18
8010ee18:  sra v0,v0,0x18
8010ee1c:  slti a0,zero,0x3b94
8010ee20:  addiu v0,v0,-0x30
8010ee24:  andi v0,v1,0x3
8010ee28:  beq v0,zero,0x8010ee48
8010ee2c:  _sll a0,a0,0x18
8010ee30:  jal 0x80052d18
8010ee34:  _sra a0,a0,0x18
8010ee38:  sll v0,v0,0x18
8010ee3c:  sra v0,v0,0x18
8010ee40:  slti a0,zero,0x3b94
8010ee44:  addiu v0,v0,-0x57
8010ee48:  lui v0,0x98
8010ee4c:  ori v0,v0,0x967f
8010ee50:  lw ra,0x10(sp)
8010ee54:  addiu sp,sp,0x18
8010ee58:  jr ra
8010ee5c:  _nop
