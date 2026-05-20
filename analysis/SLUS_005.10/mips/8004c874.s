# addr: 0x8004c874  name: MSC02_OBJ_100
8004c874:  ldsv_ t0,t1,t2
8004c880:  nop
8004c884:  SQR0 0x0
8004c888:  read_mt t3,t4,t5
8004c894:  add t3,t3,t4
8004c898:  add v0,t3,t5
8004c89c:  gte_ldLZCS v0
8004c8a0:  nop
8004c8a4:  nop
8004c8a8:  gte_stLZCR v1
8004c8ac:  li at,-0x2
8004c8b0:  and v1,v1,at
8004c8b4:  li t6,0x1f
8004c8b8:  sub t6,t6,v1
8004c8bc:  sra t6,t6,0x1
8004c8c0:  addi t3,v1,-0x18
8004c8c4:  bltz t3,0x8004c8d4
8004c8c8:  _nop
8004c8cc:  b 0x8004c8e0
8004c8d0:  _sllv t4,v0,t3
8004c8d4:  li t3,0x18
8004c8d8:  sub t3,t3,v1
8004c8dc:  srav t4,v0,t3
8004c8e0:  addi t4,t4,-0x40
8004c8e4:  sll t4,t4,0x1
8004c8e8:  lui t5,0x8006
8004c8ec:  addu t5,t5,t4
8004c8f0:  lh t5,0x628(t5)
8004c8f4:  nop
8004c8f8:  gte_ldIR0 t5
8004c8fc:  ldsv_ t0,t1,t2
8004c908:  nGPF0 0x0
8004c914:  read_mt t0,t1,t2
8004c920:  srav t0,t0,t6
8004c924:  srav t1,t1,t6
8004c928:  srav t2,t2,t6
8004c92c:  jr ra
8004c930:  _nop
