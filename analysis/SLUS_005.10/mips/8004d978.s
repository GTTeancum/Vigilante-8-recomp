# addr: 0x8004d978  name: FGO_05_OBJ_64
8004d978:  lh t2,0x0(a1)
8004d97c:  lh t5,0xc(a1)
8004d980:  multu t0,t2
8004d984:  lh t3,0x2(a1)
8004d988:  lh t6,0xe(a1)
8004d98c:  mflo t8
8004d990:  lh t4,0x4(a1)
8004d994:  lh t7,0x10(a1)
8004d998:  multu t1,t5
8004d99c:  nop
8004d9a0:  nop
8004d9a4:  mflo t9
8004d9a8:  subu t9,t8,t9
8004d9ac:  sra t8,t9,0xc
8004d9b0:  multu t0,t3
8004d9b4:  sh t8,0x0(a1)
8004d9b8:  nop
8004d9bc:  mflo t8
8004d9c0:  nop
8004d9c4:  nop
8004d9c8:  multu t1,t6
8004d9cc:  nop
8004d9d0:  nop
8004d9d4:  mflo t9
8004d9d8:  subu t9,t8,t9
8004d9dc:  sra t8,t9,0xc
8004d9e0:  multu t0,t4
8004d9e4:  sh t8,0x2(a1)
8004d9e8:  nop
8004d9ec:  mflo t8
8004d9f0:  nop
8004d9f4:  nop
8004d9f8:  multu t1,t7
8004d9fc:  nop
8004da00:  nop
8004da04:  mflo t9
8004da08:  subu t9,t8,t9
8004da0c:  sra t8,t9,0xc
8004da10:  multu t1,t2
8004da14:  sh t8,0x4(a1)
8004da18:  nop
8004da1c:  mflo t8
8004da20:  nop
8004da24:  nop
8004da28:  multu t0,t5
8004da2c:  nop
8004da30:  nop
8004da34:  mflo t9
8004da38:  addu t9,t8,t9
8004da3c:  sra t8,t9,0xc
8004da40:  multu t1,t3
8004da44:  sh t8,0xc(a1)
8004da48:  nop
8004da4c:  mflo t8
8004da50:  nop
8004da54:  nop
8004da58:  multu t0,t6
8004da5c:  nop
8004da60:  nop
8004da64:  mflo t9
8004da68:  addu t9,t8,t9
8004da6c:  sra t8,t9,0xc
8004da70:  multu t1,t4
8004da74:  sh t8,0xe(a1)
8004da78:  nop
8004da7c:  mflo t8
8004da80:  nop
8004da84:  nop
8004da88:  multu t0,t7
8004da8c:  nop
8004da90:  nop
8004da94:  mflo t9
8004da98:  addu t9,t8,t9
8004da9c:  sra t8,t9,0xc
8004daa0:  sh t8,0x10(a1)
8004daa4:  jr ra
8004daa8:  _nop
