# addr: 0x8004b230  name: CdReadBreak
8004b230:  addiu sp,sp,-0x20
8004b234:  sw s0,0x10(sp)
8004b238:  lui s0,0x8006
8004b23c:  addiu s0,s0,0x3c8
8004b240:  sw ra,0x18(sp)
8004b244:  sw s1,0x14(sp)
8004b248:  lw v0,0x0(s0)
8004b24c:  nop
8004b250:  andi v0,v0,0x1
8004b254:  beq v0,zero,0x8004b268
8004b258:  _addiu s1,s0,-0x30
8004b25c:  jal 0x80049410
8004b260:  _clear a0
8004b264:  addiu s1,s0,-0x30
8004b268:  sw zero,0x14(s1)
8004b26c:  lw a0,-0xc(s0)
8004b270:  jal 0x80048fa8
8004b274:  _nop
8004b278:  lw a0,-0x8(s0)
8004b27c:  jal 0x80048fbc
8004b280:  _nop
8004b284:  lw v0,0x0(s0)
8004b288:  nop
8004b28c:  andi v0,v0,0x1
8004b290:  beq v0,zero,0x8004b2a8
8004b294:  _li a0,0x9
8004b298:  lw a0,0x2c(s1)
8004b29c:  jal 0x800493ec
8004b2a0:  _nop
8004b2a4:  li a0,0x9
8004b2a8:  jal 0x8004910c
8004b2ac:  _clear a1
8004b2b0:  lw ra,0x18(sp)
8004b2b4:  lw s1,0x14(sp)
8004b2b8:  lw s0,0x10(sp)
8004b2bc:  jr ra
8004b2c0:  _addiu sp,sp,0x20
