# addr: 0x8004c544  name: SetFogNearFar
8004c544:  addiu sp,sp,-0x18
8004c548:  subu a3,a1,a0
8004c54c:  slti v0,a3,0x64
8004c550:  sw ra,0x14(sp)
8004c554:  bne v0,zero,0x8004c638
8004c558:  _sw s0,0x10(sp)
8004c55c:  subu v0,zero,a0
8004c560:  mult v0,a1
8004c564:  mflo v0
8004c568:  nop
8004c56c:  nop
8004c570:  div v0,a3
8004c574:  bne a3,zero,0x8004c580
8004c578:  _nop
8004c57c:  break 0x1c00
8004c580:  li at,-0x1
8004c584:  bne a3,at,0x8004c598
8004c588:  _lui at,0x8000
8004c58c:  bne v0,at,0x8004c598
8004c590:  _nop
8004c594:  break 0x1800
8004c598:  mflo v0
8004c59c:  sll v1,a1,0xc
8004c5a0:  nop
8004c5a4:  div v1,a3
8004c5a8:  bne a3,zero,0x8004c5b4
8004c5ac:  _nop
8004c5b0:  break 0x1c00
8004c5b4:  li at,-0x1
8004c5b8:  bne a3,at,0x8004c5cc
8004c5bc:  _lui at,0x8000
8004c5c0:  bne v1,at,0x8004c5cc
8004c5c4:  _nop
8004c5c8:  break 0x1800
8004c5cc:  mflo v1
8004c5d0:  sll v0,v0,0x8
8004c5d4:  nop
8004c5d8:  div v0,a2
8004c5dc:  bne a2,zero,0x8004c5e8
8004c5e0:  _nop
8004c5e4:  break 0x1c00
8004c5e8:  li at,-0x1
8004c5ec:  bne a2,at,0x8004c600
8004c5f0:  _lui at,0x8000
8004c5f4:  bne v0,at,0x8004c600
8004c5f8:  _nop
8004c5fc:  break 0x1800
8004c600:  mflo a0
8004c604:  slti v0,a0,-0x8000
8004c608:  beq v0,zero,0x8004c614
8004c60c:  _sll s0,v1,0xc
8004c610:  li a0,-0x8000
8004c614:  li v0,0x7fff
8004c618:  slt v0,v0,a0
8004c61c:  beq v0,zero,0x8004c628
8004c620:  _nop
8004c624:  li a0,0x7fff
8004c628:  jal 0x8004d4cc
8004c62c:  _nop
8004c630:  jal 0x8004d4d8
8004c634:  _move a0,s0
8004c638:  lw ra,0x14(sp)
8004c63c:  lw s0,0x10(sp)
8004c640:  jr ra
8004c644:  _addiu sp,sp,0x18
