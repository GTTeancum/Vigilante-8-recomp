# addr: 0x8010a070  name: FUN_8010a070
8010a070:  addiu sp,sp,-0x38
8010a074:  addiu a0,sp,0x18
8010a078:  li v0,0x1e0
8010a07c:  clear a1
8010a080:  move a2,a1
8010a084:  li a3,0x280
8010a088:  sw ra,0x30(sp)
8010a08c:  jal 0x8004f198
8010a090:  _sw v0,0x10(sp)
8010a094:  lui v0,0x8006
8010a098:  lbu v1,0x531c(v0)
8010a09c:  lui v0,0x8006
8010a0a0:  lbu v0,0x531d(v0)
8010a0a4:  addiu a0,sp,0x18
8010a0a8:  sll v1,v1,0x18
8010a0ac:  sra v1,v1,0x18
8010a0b0:  sll v0,v0,0x18
8010a0b4:  sra v0,v0,0x18
8010a0b8:  sh v1,0x20(sp)
8010a0bc:  jal 0x8004fdb0
8010a0c0:  _sh v0,0x22(sp)
8010a0c4:  lw ra,0x30(sp)
8010a0c8:  nop
8010a0cc:  jr ra
8010a0d0:  _addiu sp,sp,0x38
