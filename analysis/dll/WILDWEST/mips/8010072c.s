# addr: 0x8010072c  name: FUN_8010072c
8010072c:  addiu sp,sp,-0x38
80100730:  sw s0,0x28(sp)
80100734:  move s0,a0
80100738:  sw ra,0x30(sp)
8010073c:  sw s1,0x2c(sp)
80100740:  lw v0,0xa4(s0)
80100744:  nop
80100748:  beq v0,zero,0x80100928
8010074c:  _lui v1,0x1
80100750:  lw v0,0x0(s0)
80100754:  nop
80100758:  and v0,v0,v1
8010075c:  beq v0,zero,0x80100780
80100760:  _nop
80100764:  lw v0,0xa8(s0)
80100768:  nop
8010076c:  addiu v0,v0,-0xe
80100770:  blez v0,0x8010077c
80100774:  _clear v1
80100778:  move v1,v0
8010077c:  sw v1,0xa8(s0)
80100780:  lw a1,0xb0(s0)
80100784:  nop
80100788:  bgez a1,0x80100794
8010078c:  _ori v0,zero,0xffff
80100790:  addu a1,a1,v0
80100794:  lw a0,0xa4(s0)
80100798:  sra a1,a1,0x10
8010079c:  addiu a2,s0,0x24
801007a0:  jal 0x80042390
801007a4:  _addiu a3,sp,0x10
801007a8:  sw zero,0x14(sp)
801007ac:  lw v0,0xac(s0)
801007b0:  nop
801007b4:  bne v0,zero,0x801007d4
801007b8:  _nop
801007bc:  lw v0,0x10(sp)
801007c0:  lw v1,0x18(sp)
801007c4:  subu v0,zero,v0
801007c8:  subu v1,zero,v1
801007cc:  sw v0,0x10(sp)
801007d0:  sw v1,0x18(sp)
801007d4:  addiu a0,sp,0x10
801007d8:  jal 0x8004c800
801007dc:  _addiu a1,sp,0x20
801007e0:  lhu v0,0x24(sp)
801007e4:  nop
801007e8:  sh v0,0x20(s0)
801007ec:  sh v0,0x10(s0)
801007f0:  lhu v0,0x20(sp)
801007f4:  nop
801007f8:  sh v0,0x14(s0)
801007fc:  subu v0,zero,v0
80100800:  sh v0,0x1c(s0)
80100804:  lw v0,0x10(sp)
80100808:  nop
8010080c:  mult v0,v0
80100810:  lw v0,0x18(sp)
80100814:  mflo v1
80100818:  nop
8010081c:  nop
80100820:  mult v0,v0
80100824:  mflo t1
80100828:  jal 0x8004c6e4
8010082c:  _addu a0,v1,t1
80100830:  lw v1,0xac(s0)
80100834:  lw a1,0xb0(s0)
80100838:  beq v1,zero,0x8010084c
8010083c:  _move a0,v0
80100840:  lw v0,0xa8(s0)
80100844:  slti a0,zero,0x217
80100848:  sll v0,v0,0x10
8010084c:  lw v0,0xa8(s0)
80100850:  nop
80100854:  subu v0,zero,v0
80100858:  sll v0,v0,0x10
8010085c:  div v0,a0
80100860:  mflo v0
80100864:  nop
80100868:  addu v0,a1,v0
8010086c:  sw v0,0xb0(s0)
80100870:  lui s1,0x1000
80100874:  sltu v0,s1,v0
80100878:  beq v0,zero,0x8010093c
8010087c:  _clear v0
80100880:  lw a0,0xa4(s0)
80100884:  jal 0x80042698
80100888:  _addiu a1,s0,0xac
8010088c:  beq v0,zero,0x801008b0
80100890:  _sw v0,0xa4(s0)
80100894:  lw v0,0xac(s0)
80100898:  nop
8010089c:  sltiu v0,v0,0x1
801008a0:  subu v0,zero,v0
801008a4:  and v0,v0,s1
801008a8:  slti a0,zero,0x248
801008ac:  sw v0,0xb0(s0)
801008b0:  lh v1,0x14(s0)
801008b4:  lw v0,0xa8(s0)
801008b8:  nop
801008bc:  mult v1,v0
801008c0:  lw v0,0x0(s0)
801008c4:  addiu a1,s0,0x80
801008c8:  li v1,-0x101
801008cc:  and v0,v0,v1
801008d0:  mflo a0
801008d4:  bgez a0,0x801008e0
801008d8:  _sw v0,0x0(s0)
801008dc:  addiu a0,a0,0x1f
801008e0:  sra v0,a0,0x5
801008e4:  sw v0,0x80(s0)
801008e8:  sw zero,0x4(a1)
801008ec:  lh v1,0x20(s0)
801008f0:  lw v0,0xa8(s0)
801008f4:  nop
801008f8:  mult v1,v0
801008fc:  mflo v0
80100900:  bgez v0,0x8010090c
80100904:  _nop
80100908:  addiu v0,v0,0x1f
8010090c:  sra v0,v0,0x5
80100910:  sw v0,0x8(a1)
80100914:  move a0,s0
80100918:  jal 0x80020890
8010091c:  _li a1,0x78
80100920:  slti a0,zero,0x24f
80100924:  li v0,0x1
80100928:  lw a1,0x5c(s0)
8010092c:  move a0,s0
80100930:  jal 0x8001787c
80100934:  _addiu a1,a1,0x4
80100938:  clear v0
8010093c:  lw ra,0x30(sp)
80100940:  lw s1,0x2c(sp)
80100944:  lw s0,0x28(sp)
80100948:  jr ra
8010094c:  _addiu sp,sp,0x38
