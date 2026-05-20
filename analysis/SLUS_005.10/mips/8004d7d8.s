# addr: 0x8004d7d8  name: FGO_04_OBJ_64
8004d7d8:  lh t2,0x6(a1)
8004d7dc:  lh t5,0xc(a1)
8004d7e0:  multu t0,t2
8004d7e4:  lh t3,0x8(a1)
8004d7e8:  lh t6,0xe(a1)
8004d7ec:  mflo t8
8004d7f0:  lh t4,0xa(a1)
8004d7f4:  lh t7,0x10(a1)
8004d7f8:  multu t1,t5
8004d7fc:  nop
8004d800:  nop
8004d804:  mflo t9
8004d808:  subu t9,t8,t9
8004d80c:  sra t8,t9,0xc
8004d810:  multu t0,t3
8004d814:  sh t8,0x6(a1)
8004d818:  nop
8004d81c:  mflo t8
8004d820:  nop
8004d824:  nop
8004d828:  multu t1,t6
8004d82c:  nop
8004d830:  nop
8004d834:  mflo t9
8004d838:  subu t9,t8,t9
8004d83c:  sra t8,t9,0xc
8004d840:  multu t0,t4
8004d844:  sh t8,0x8(a1)
8004d848:  nop
8004d84c:  mflo t8
8004d850:  nop
8004d854:  nop
8004d858:  multu t1,t7
8004d85c:  nop
8004d860:  nop
8004d864:  mflo t9
8004d868:  subu t9,t8,t9
8004d86c:  sra t8,t9,0xc
8004d870:  multu t1,t2
8004d874:  sh t8,0xa(a1)
8004d878:  nop
8004d87c:  mflo t8
8004d880:  nop
8004d884:  nop
8004d888:  multu t0,t5
8004d88c:  nop
8004d890:  nop
8004d894:  mflo t9
8004d898:  addu t9,t8,t9
8004d89c:  sra t8,t9,0xc
8004d8a0:  multu t1,t3
8004d8a4:  sh t8,0xc(a1)
8004d8a8:  nop
8004d8ac:  mflo t8
8004d8b0:  nop
8004d8b4:  nop
8004d8b8:  multu t0,t6
8004d8bc:  nop
8004d8c0:  nop
8004d8c4:  mflo t9
8004d8c8:  addu t9,t8,t9
8004d8cc:  sra t8,t9,0xc
8004d8d0:  multu t1,t4
8004d8d4:  sh t8,0xe(a1)
8004d8d8:  nop
8004d8dc:  mflo t8
8004d8e0:  nop
8004d8e4:  nop
8004d8e8:  multu t0,t7
8004d8ec:  nop
8004d8f0:  nop
8004d8f4:  mflo t9
8004d8f8:  addu t9,t8,t9
8004d8fc:  sra t8,t9,0xc
8004d900:  sh t8,0x10(a1)
8004d904:  jr ra
8004d908:  _nop
