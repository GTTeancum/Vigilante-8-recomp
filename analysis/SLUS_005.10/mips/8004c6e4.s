# addr: 0x8004c6e4  name: SquareRoot0
8004c6e4:  gte_ldLZCS a0
8004c6e8:  nop
8004c6ec:  nop
8004c6f0:  gte_stLZCR v0
8004c6f4:  li at,0x20
8004c6f8:  beq v0,at,0x8004c760
8004c6fc:  _nop
8004c700:  andi t0,v0,0x1
8004c704:  li at,-0x2
8004c708:  and t2,v0,at
8004c70c:  li t1,0x1f
8004c710:  sub t1,t1,t2
8004c714:  sra t1,t1,0x1
8004c718:  addi t3,t2,-0x18
8004c71c:  bltz t3,0x8004c72c
8004c720:  _nop
8004c724:  sllv t4,a0,t3
8004c728:  b 0x8004c738
8004c72c:  _li t3,0x18
8004c738:  addi t4,t4,-0x40
8004c73c:  sll t4,t4,0x1
8004c740:  lui t5,0x8006
8004c744:  addu t5,t5,t4
8004c748:  lh t5,0x494(t5)
8004c74c:  nop
8004c750:  sllv t5,t5,t1
8004c754:  srl v0,t5,0xc
8004c758:  jr ra
8004c75c:  _nop
8004c760:  jr ra
8004c764:  _li v0,0x0
