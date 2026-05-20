# addr: 0x800176f8  name: FUN_800176f8
800176f8:  addiu sp,sp,-0x20
800176fc:  sw s0,0x18(sp)
80017700:  move s0,a0
80017704:  sw ra,0x1c(sp)
80017708:  lw v0,0x80(s0)
8001770c:  lw v1,0x0(a1)
80017710:  addiu a0,s0,0x80
80017714:  addu v0,v0,v1
80017718:  sw v0,0x80(s0)
8001771c:  lw v0,0x4(a0)
80017720:  lw a3,0x4(a1)
80017724:  lw v1,0x8(a0)
80017728:  addu v0,v0,a3
8001772c:  sw v0,0x4(a0)
80017730:  lw v0,0x8(a1)
80017734:  nop
80017738:  addu v1,v1,v0
8001773c:  sw v1,0x8(a0)
80017740:  lw a0,0x0(a2)
80017744:  lw a3,0x48(s0)
80017748:  lw v1,0x4(a2)
8001774c:  lw t0,0x4c(s0)
80017750:  lw v0,0x8(a2)
80017754:  lw a2,0x50(s0)
80017758:  subu a0,a0,a3
8001775c:  sra a0,a0,0x3
80017760:  subu v1,v1,t0
80017764:  sra v1,v1,0x3
80017768:  subu v0,v0,a2
8001776c:  sra v0,v0,0x3
80017770:  gte_ldR11R12 a0
80017774:  gte_ldR22R23 v1
80017778:  gte_ldR33 v0
8001777c:  lw a0,0x0(a1)
80017780:  lw v1,0x4(a1)
80017784:  lw v0,0x8(a1)
80017788:  sra a0,a0,0x3
8001778c:  sra v1,v1,0x3
80017790:  sra v0,v0,0x3
80017794:  ldsv_ a0,v1,v0
800177a0:  nOP12
800177ac:  gte_stMAC1 v0
800177b0:  nop
800177b4:  sh v0,0x10(sp)
800177b8:  gte_stMAC2 v0
800177bc:  nop
800177c0:  sh v0,0x12(sp)
800177c4:  gte_stMAC3 v0
800177c8:  addiu a0,s0,0x10
800177cc:  addiu a1,sp,0x10
800177d0:  move a2,a1
800177d4:  jal 0x800434f8
800177d8:  _sh v0,0x14(sp)
800177dc:  lh v1,0x10(sp)
800177e0:  lh v0,0x9c(s0)
800177e4:  nop
800177e8:  mult v1,v0
800177ec:  mflo v1
800177f0:  bgez v1,0x800177fc
800177f4:  _nop
800177f8:  addiu v1,v1,0x7f
800177fc:  lw v0,0x90(s0)
80017800:  sra v1,v1,0x7
80017804:  addu v0,v0,v1
80017808:  sw v0,0x90(s0)
8001780c:  lh v1,0x12(sp)
80017810:  lh v0,0x9e(s0)
80017814:  nop
80017818:  mult v1,v0
8001781c:  mflo v1
80017820:  bgez v1,0x8001782c
80017824:  _nop
80017828:  addiu v1,v1,0x7f
8001782c:  lw v0,0x94(s0)
80017830:  sra v1,v1,0x7
80017834:  addu v0,v0,v1
80017838:  sw v0,0x94(s0)
8001783c:  lh v1,0x14(sp)
80017840:  lh v0,0xa0(s0)
80017844:  nop
80017848:  mult v1,v0
8001784c:  mflo v1
80017850:  bgez v1,0x8001785c
80017854:  _nop
80017858:  addiu v1,v1,0x7f
8001785c:  lw v0,0x98(s0)
80017860:  sra v1,v1,0x7
80017864:  addu v0,v0,v1
80017868:  sw v0,0x98(s0)
8001786c:  lw ra,0x1c(sp)
80017870:  lw s0,0x18(sp)
80017874:  jr ra
80017878:  _addiu sp,sp,0x20
