# addr: 0x80052edc  name: putchar
80052edc:  addiu sp,sp,-0x18
80052ee0:  sw s0,0x10(sp)
80052ee4:  move s0,a0
80052ee8:  sll a0,a0,0x18
80052eec:  sra a0,a0,0x18
80052ef0:  li v0,0x9
80052ef4:  beq a0,v0,0x80052f1c
80052ef8:  _sw ra,0x14(sp)
80052efc:  li v0,0xa
80052f00:  bne a0,v0,0x80052f44
80052f04:  _andi v0,s0,0xff
80052f08:  jal 0x80052da4
80052f0c:  _li a0,0xd
80052f10:  lui at,0x8006
80052f14:  j 0x80052f78
80052f18:  _sw zero,0x5204(at)
