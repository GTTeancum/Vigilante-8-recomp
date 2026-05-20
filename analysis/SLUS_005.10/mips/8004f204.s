# addr: 0x8004f204  name: ResetGraph
8004f204:  addiu sp,sp,-0x20
8004f208:  sw s1,0x14(sp)
8004f20c:  move s1,a0
8004f210:  andi v1,s1,0x7
8004f214:  li v0,0x3
8004f218:  sw ra,0x18(sp)
8004f21c:  beq v1,v0,0x8004f250
8004f220:  _sw s0,0x10(sp)
8004f224:  slti v0,v1,0x4
8004f228:  beq v0,zero,0x8004f240
8004f22c:  _li v0,0x5
8004f230:  beq v1,zero,0x8004f250
8004f234:  _nop
8004f238:  j 0x8004f314
8004f23c:  _nop
8004f240:  beq v1,v0,0x8004f26c
8004f244:  _nop
8004f248:  j 0x8004f314
8004f24c:  _nop
8004f250:  lui a0,0x8001
8004f254:  addiu a0,a0,0x1198
8004f258:  lui a1,0x8006
8004f25c:  addiu a1,a1,0x4fdc
8004f260:  lui a2,0x8006
8004f264:  jal 0x80052604
8004f268:  _addiu a2,a2,0x5024
8004f26c:  lui s0,0x8006
8004f270:  addiu s0,s0,0x5024
8004f274:  move a0,s0
8004f278:  clear a1
8004f27c:  jal 0x800521e8
8004f280:  _li a2,0x80
8004f284:  jal 0x80048054
8004f288:  _nop
8004f28c:  lui v0,0xff
8004f290:  lui a0,0x8006
8004f294:  lw a0,0x501c(a0)
8004f298:  ori v0,v0,0xffff
8004f29c:  jal 0x80053974
8004f2a0:  _and a0,a0,v0
8004f2a4:  jal 0x80051904
8004f2a8:  _move a0,s1
8004f2ac:  addiu a0,s0,0x10
8004f2b0:  sb v0,0x0(s0)
8004f2b4:  lbu v0,0x0(s0)
8004f2b8:  li v1,0x1
8004f2bc:  sb v1,0x1(s0)
8004f2c0:  sll v0,v0,0x2
8004f2c4:  lui v1,0x8006
8004f2c8:  addu v1,v1,v0
8004f2cc:  lhu v1,0x50a4(v1)
8004f2d0:  lbu v0,0x0(s0)
8004f2d4:  li a1,-0x1
8004f2d8:  sll v0,v0,0x2
8004f2dc:  sh v1,0x4(s0)
8004f2e0:  lui at,0x8006
8004f2e4:  addu at,at,v0
8004f2e8:  lhu v0,0x50b0(at)
8004f2ec:  li a2,0x5c
8004f2f0:  jal 0x800521e8
8004f2f4:  _sh v0,0x6(s0)
8004f2f8:  addiu a0,s0,0x6c
8004f2fc:  li a1,-0x1
8004f300:  jal 0x800521e8
8004f304:  _li a2,0x14
8004f308:  lbu v0,0x0(s0)
8004f30c:  j 0x8004f364
8004f310:  _nop
