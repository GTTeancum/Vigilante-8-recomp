# addr: 0x8001b1f8  name: FUN_8001b1f8
8001b1f8:  lw v0,0x58(a0)
8001b1fc:  lhu v1,0xa(a0)
8001b200:  lw a0,0x0(v0)
8001b204:  sll v0,v1,0x3
8001b208:  subu v0,v0,v1
8001b20c:  sll v0,v0,0x2
8001b210:  addu v0,a0,v0
8001b214:  lhu v1,0x36(v0)
8001b218:  ori v0,zero,0xffff
8001b21c:  beq v1,v0,0x8001b260
8001b220:  _li a2,0xb
8001b224:  move a1,v0
8001b228:  sll v0,v1,0x3
8001b22c:  subu v0,v0,v1
8001b230:  sll v0,v0,0x2
8001b234:  addiu v0,v0,0x1c
8001b238:  addu v1,a0,v0
8001b23c:  lhu v0,0x0(v1)
8001b240:  nop
8001b244:  srl v0,v0,0xc
8001b248:  beq v0,a2,0x8001b268
8001b24c:  _move v0,v1
8001b250:  lhu v1,0x18(v1)
8001b254:  nop
8001b258:  bne v1,a1,0x8001b22c
8001b25c:  _sll v0,v1,0x3
8001b260:  jr ra
8001b264:  _clear v0
8001b268:  jr ra
8001b26c:  _nop
