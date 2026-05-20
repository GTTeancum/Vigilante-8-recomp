# addr: 0x8004c774  name: InvSquareRoot
8004c774:  gte_ldLZCS a0
8004c778:  nop
8004c77c:  nop
8004c780:  gte_stLZCR v0
8004c784:  li at,0x20
8004c788:  beq v0,at,0x8004c7f8
8004c78c:  _nop
8004c790:  beq v0,zero,0x8004c7f8
8004c794:  _nop
8004c798:  andi t0,v0,0x1
8004c79c:  li at,-0x2
8004c7a0:  and t2,v0,at
8004c7a4:  li t1,0x1f
8004c7a8:  sub t1,t1,t2
8004c7ac:  sra t1,t1,0x1
8004c7b0:  addi t3,t2,-0x18
8004c7b4:  bltz t3,0x8004c7c4
8004c7b8:  _nop
8004c7bc:  sllv t4,a0,t3
8004c7c0:  b 0x8004c7d0
8004c7c4:  _li t3,0x18
8004c7d0:  addi t4,t4,-0x40
8004c7d4:  sll t4,t4,0x1
8004c7d8:  lui t5,0x8006
8004c7dc:  addu t5,t5,t4
8004c7e0:  lh t5,0x628(t5)
8004c7e4:  sw t1,0x0(a2)
8004c7e8:  sw t5,0x0(a1)
8004c7ec:  li v0,0x1
8004c7f0:  jr ra
8004c7f4:  _nop
8004c7f8:  jr ra
8004c7fc:  _li v0,-0x1
