# addr: 0x8004f4e8  name: SetDispMask
8004f4e8:  addiu sp,sp,-0x20
8004f4ec:  sw s1,0x14(sp)
8004f4f0:  lui s1,0x8006
8004f4f4:  addiu s1,s1,0x5026
8004f4f8:  sw ra,0x18(sp)
8004f4fc:  sw s0,0x10(sp)
8004f500:  lbu v0,0x0(s1)
8004f504:  nop
8004f508:  sltiu v0,v0,0x2
8004f50c:  bne v0,zero,0x8004f530
8004f510:  _move s0,a0
8004f514:  lui a0,0x8001
8004f518:  addiu a0,a0,0x1228
8004f51c:  lui v0,0x8006
8004f520:  lw v0,0x5020(v0)
8004f524:  nop
8004f528:  jalr v0
8004f52c:  _move a1,s0
8004f530:  bne s0,zero,0x8004f544
8004f534:  _addiu a0,s1,0x6a
8004f538:  li a1,-0x1
8004f53c:  jal 0x800521e8
8004f540:  _li a2,0x14
8004f544:  lui a0,0x300
8004f548:  lui v0,0x8006
8004f54c:  lw v0,0x501c(v0)
8004f550:  beq s0,zero,0x8004f55c
8004f554:  _ori a0,a0,0x1
8004f558:  lui a0,0x300
8004f55c:  lw v0,0x10(v0)
8004f560:  nop
8004f564:  jalr v0
8004f568:  _nop
8004f56c:  lw ra,0x18(sp)
8004f570:  lw s1,0x14(sp)
8004f574:  lw s0,0x10(sp)
8004f578:  jr ra
8004f57c:  _addiu sp,sp,0x20
