# addr: 0x8004b3c4  name: CdReadSync
8004b3c4:  addiu sp,sp,-0x28
8004b3c8:  sw s2,0x18(sp)
8004b3cc:  move s2,a0
8004b3d0:  sw s3,0x1c(sp)
8004b3d4:  move s3,a1
8004b3d8:  sw s1,0x14(sp)
8004b3dc:  lui s1,0x8006
8004b3e0:  addiu s1,s1,0x3b4
8004b3e4:  sw ra,0x20(sp)
8004b3e8:  sw s0,0x10(sp)
8004b3ec:  jal 0x80047e44
8004b3f0:  _li a0,-0x1
8004b3f4:  lw v1,0x0(s1)
8004b3f8:  nop
8004b3fc:  addiu v1,v1,0x4b0
8004b400:  slt v1,v1,v0
8004b404:  bne v1,zero,0x8004b454
8004b408:  _li s0,-0x1
8004b40c:  lw v0,-0x8(s1)
8004b410:  nop
8004b414:  bltz v0,0x8004b43c
8004b418:  _nop
8004b41c:  jal 0x80047e44
8004b420:  _li a0,-0x1
8004b424:  lw v1,-0x4(s1)
8004b428:  nop
8004b42c:  addiu v1,v1,0x3c
8004b430:  slt v1,v1,v0
8004b434:  beq v1,zero,0x8004b450
8004b438:  _nop
8004b43c:  jal 0x8004b040
8004b440:  _li a0,0x1
8004b444:  lw s0,-0x1c(s1)
8004b448:  j 0x8004b454
8004b44c:  _nop
8004b450:  lw s0,-0x8(s1)
