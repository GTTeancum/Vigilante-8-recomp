# addr: 0x8002c6fc  name: FUN_8002c6fc
8002c6fc:  addiu sp,sp,-0x30
8002c700:  sw s3,0x1c(sp)
8002c704:  move s3,a0
8002c708:  sw ra,0x2c(sp)
8002c70c:  sw s6,0x28(sp)
8002c710:  sw s5,0x24(sp)
8002c714:  sw s4,0x20(sp)
8002c718:  sw s2,0x18(sp)
8002c71c:  sw s1,0x14(sp)
8002c720:  sw s0,0x10(sp)
8002c724:  lw v0,0x5c(s3)
8002c728:  nop
8002c72c:  addiu a0,v0,0x4
8002c730:  lw v0,0x14(a0)
8002c734:  move t0,a1
8002c738:  lw a1,0x8(a2)
8002c73c:  srl v1,v0,0x1f
8002c740:  addu v0,v0,v1
8002c744:  sra v0,v0,0x1
8002c748:  slt v0,v0,a1
8002c74c:  bne v0,zero,0x8002c77c
8002c750:  _move s6,a3
8002c754:  lw v0,0x8(a0)
8002c758:  li s4,0x1
8002c75c:  srl v1,v0,0x1f
8002c760:  addu v0,v0,v1
8002c764:  srav v0,v0,s4
8002c768:  slt v0,a1,v0
8002c76c:  beq v0,zero,0x8002c780
8002c770:  _nop
8002c774:  j 0x8002c780
8002c778:  _li s4,0x2
8002c77c:  clear s4
8002c780:  lhu s2,0xc(s3)
8002c784:  bgez t0,0x8002c930
8002c788:  _clear v0
8002c78c:  beq s2,zero,0x8002c930
8002c790:  _nop
8002c794:  lw v0,0x604(gp)
8002c798:  nop
8002c79c:  andi v0,v0,0x8
8002c7a0:  beq v0,zero,0x8002c7b8
8002c7a4:  _nop
8002c7a8:  lh v0,0x6(s3)
8002c7ac:  nop
8002c7b0:  bltz v0,0x8002c930
8002c7b4:  _clear v0
8002c7b8:  lbu v1,0xd0(s3)
8002c7bc:  li v0,0xc
8002c7c0:  bne v1,v0,0x8002c7fc
8002c7c4:  _addu v0,s2,t0
8002c7c8:  bgtz v0,0x8002c7dc
8002c7cc:  _nop
8002c7d0:  bne s6,zero,0x8002c8fc
8002c7d4:  _slti v0,v0,-0x14
8002c7d8:  li v0,0x1
8002c7dc:  j 0x8002c92c
8002c7e0:  _sh v0,0xc(s3)
8002c7e4:  j 0x8002c92c
8002c7e8:  _sh s0,0xc(s1)
8002c7ec:  jal 0x8002bd84
8002c7f0:  _move a0,s3
8002c7f4:  j 0x8002c930
8002c7f8:  _li v0,0x1
8002c7fc:  sll v1,s4,0x2
8002c800:  addu v0,s3,v1
8002c804:  lw v0,0xec(v0)
8002c808:  nop
8002c80c:  beq v0,zero,0x8002c92c
8002c810:  _srl s5,s2,0x1
8002c814:  addu v0,s3,v1
8002c818:  lw s1,0xec(v0)
8002c81c:  nop
8002c820:  lhu a0,0xc(s1)
8002c824:  lb a1,0x8(s1)
8002c828:  nop
8002c82c:  mult a0,a1
8002c830:  mflo v1
8002c834:  addu v1,v1,s5
8002c838:  nop
8002c83c:  div v1,s2
8002c840:  mflo v1
8002c844:  addu s0,a0,t0
8002c848:  nop
8002c84c:  mult s0,a1
8002c850:  mflo t0
8002c854:  addu v0,t0,s5
8002c858:  nop
8002c85c:  div v0,s2
8002c860:  mflo v0
8002c864:  nop
8002c868:  beq v1,v0,0x8002c878
8002c86c:  _nop
8002c870:  jal 0x8003fc50
8002c874:  _move a0,s1
8002c878:  bgez s0,0x8002c7e4
8002c87c:  _li v0,0x1
8002c880:  sh zero,0xc(s1)
8002c884:  bne s4,v0,0x8002c8a8
8002c888:  _move t0,s0
8002c88c:  lw v0,0xec(s3)
8002c890:  nop
8002c894:  lhu v0,0xc(v0)
8002c898:  nop
8002c89c:  sltiu v0,v0,0x1
8002c8a0:  j 0x8002c8ac
8002c8a4:  _sll v1,v0,0x1
8002c8a8:  li v1,0x1
8002c8ac:  lw v0,0xec(s3)
8002c8b0:  nop
8002c8b4:  lhu v0,0xc(v0)
8002c8b8:  nop
8002c8bc:  bne v0,zero,0x8002c914
8002c8c0:  _move s4,v1
8002c8c4:  lw v0,0xf0(s3)
8002c8c8:  nop
8002c8cc:  lhu v0,0xc(v0)
8002c8d0:  nop
8002c8d4:  bne v0,zero,0x8002c914
8002c8d8:  _nop
8002c8dc:  lw v0,0xf4(s3)
8002c8e0:  nop
8002c8e4:  lhu v0,0xc(v0)
8002c8e8:  nop
8002c8ec:  bne v0,zero,0x8002c914
8002c8f0:  _nop
8002c8f4:  beq s6,zero,0x8002c92c
8002c8f8:  _slti v0,t0,-0x14
8002c8fc:  bne v0,zero,0x8002c7ec
8002c900:  _nop
8002c904:  jal 0x8002be84
8002c908:  _move a0,s3
8002c90c:  j 0x8002c930
8002c910:  _clear v0
8002c914:  sll v1,s4,0x2
8002c918:  addu v0,s3,v1
8002c91c:  lw v0,0xec(v0)
8002c920:  nop
8002c924:  bne v0,zero,0x8002c818
8002c928:  _addu v0,s3,v1
8002c92c:  clear v0
8002c930:  lw ra,0x2c(sp)
8002c934:  lw s6,0x28(sp)
8002c938:  lw s5,0x24(sp)
8002c93c:  lw s4,0x20(sp)
8002c940:  lw s3,0x1c(sp)
8002c944:  lw s2,0x18(sp)
8002c948:  lw s1,0x14(sp)
8002c94c:  lw s0,0x10(sp)
8002c950:  jr ra
8002c954:  _addiu sp,sp,0x30
