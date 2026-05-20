# addr: 0x8002c4bc  name: FUN_8002c4bc
8002c4bc:  addiu sp,sp,-0x18
8002c4c0:  sw s0,0x10(sp)
8002c4c4:  move s0,a0
8002c4c8:  sw ra,0x14(sp)
8002c4cc:  lw a0,0x0(s0)
8002c4d0:  lui a1,0x800
8002c4d4:  and v0,a0,a1
8002c4d8:  bne v0,zero,0x8002c58c
8002c4dc:  _li v0,0xc
8002c4e0:  lbu v1,0xd0(s0)
8002c4e4:  nop
8002c4e8:  beq v1,v0,0x8002c58c
8002c4ec:  _or v0,a0,a1
8002c4f0:  jal 0x80017160
8002c4f4:  _sw v0,0x0(s0)
8002c4f8:  sll v1,v0,0x1
8002c4fc:  addu v1,v1,v0
8002c500:  lb a0,0x5(s0)
8002c504:  sll v0,v1,0x4
8002c508:  subu v0,v0,v1
8002c50c:  sll v0,v0,0x2
8002c510:  sra v0,v0,0xf
8002c514:  addiu v0,v0,0xb4
8002c518:  sh v0,0xb0(s0)
8002c51c:  li v0,-0x78
8002c520:  beq a0,zero,0x8002c530
8002c524:  _sh v0,0xa6(s0)
8002c528:  j 0x8002c538
8002c52c:  _move v0,a0
8002c530:  jal 0x8004410c
8002c534:  _nop
8002c538:  lw a1,0x5f8(gp)
8002c53c:  move a0,v0
8002c540:  li a2,0x20
8002c544:  jal 0x8004483c
8002c548:  _addiu a3,s0,0x24
8002c54c:  lh v0,0x6(s0)
8002c550:  nop
8002c554:  bltz v0,0x8002c588
8002c558:  _nop
8002c55c:  jal 0x8004410c
8002c560:  _nop
8002c564:  sb v0,0x5(s0)
8002c568:  lw a1,0x5f8(gp)
8002c56c:  sll v0,v0,0x18
8002c570:  sra a0,v0,0x18
8002c574:  li a2,0x21
8002c578:  jal 0x800443c8
8002c57c:  _clear a3
8002c580:  j 0x8002c58c
8002c584:  _nop
8002c588:  sb zero,0x5(s0)
8002c58c:  lw ra,0x14(sp)
8002c590:  lw s0,0x10(sp)
8002c594:  jr ra
8002c598:  _addiu sp,sp,0x18
