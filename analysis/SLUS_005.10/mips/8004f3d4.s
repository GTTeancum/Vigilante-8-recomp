# addr: 0x8004f3d4  name: SetGraphQueue
8004f3d4:  addiu sp,sp,-0x20
8004f3d8:  sw s0,0x10(sp)
8004f3dc:  sw s1,0x14(sp)
8004f3e0:  lui s1,0x8006
8004f3e4:  addiu s1,s1,0x5025
8004f3e8:  sw ra,0x1c(sp)
8004f3ec:  sw s2,0x18(sp)
8004f3f0:  lbu v0,0x1(s1)
8004f3f4:  lbu s2,0x0(s1)
8004f3f8:  sltiu v0,v0,0x2
8004f3fc:  bne v0,zero,0x8004f420
8004f400:  _move s0,a0
8004f404:  lui a0,0x8001
8004f408:  addiu a0,a0,0x11f8
8004f40c:  lui v0,0x8006
8004f410:  lw v0,0x5020(v0)
8004f414:  nop
8004f418:  jalr v0
8004f41c:  _move a1,s0
8004f420:  lbu v0,0x0(s1)
8004f424:  nop
8004f428:  beq s0,v0,0x8004f460
8004f42c:  _move v0,s2
8004f430:  lui v0,0x8006
8004f434:  lw v0,0x501c(v0)
8004f438:  nop
8004f43c:  lw v0,0x34(v0)
8004f440:  nop
8004f444:  jalr v0
8004f448:  _li a0,0x1
8004f44c:  li a0,0x2
8004f450:  clear a1
8004f454:  jal 0x800480b4
8004f458:  _sb s0,0x0(s1)
8004f45c:  move v0,s2
8004f460:  lw ra,0x1c(sp)
8004f464:  lw s2,0x18(sp)
8004f468:  lw s1,0x14(sp)
8004f46c:  lw s0,0x10(sp)
8004f470:  jr ra
8004f474:  _addiu sp,sp,0x20
