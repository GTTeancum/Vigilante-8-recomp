# addr: 0x8010068c  name: FUN_8010068c
8010068c:  addiu sp,sp,-0x30
80100690:  sw s1,0x14(sp)
80100694:  move s1,a0
80100698:  sw s5,0x24(sp)
8010069c:  move s5,a2
801006a0:  sw ra,0x28(sp)
801006a4:  sw s4,0x20(sp)
801006a8:  sw s3,0x1c(sp)
801006ac:  sw s2,0x18(sp)
801006b0:  beq a1,zero,0x801006cc
801006b4:  _sw s0,0x10(sp)
801006b8:  li v0,0x2
801006bc:  beq a1,v0,0x80100a9c
801006c0:  _nop
801006c4:  slti a0,zero,0x2fc
801006c8:  nop
801006cc:  lb v0,0x8(s1)
801006d0:  nop
801006d4:  beq v0,zero,0x8010094c
801006d8:  _nop
801006dc:  lw s0,0xa4(s1)
801006e0:  lw v0,0x48(s1)
801006e4:  lw v1,0x50(s1)
801006e8:  lw a0,0x48(s0)
801006ec:  lw a1,0x50(s0)
801006f0:  subu s3,a0,v0
801006f4:  move a0,s3
801006f8:  subu s4,a1,v1
801006fc:  jal 0x8004ecd4
80100700:  _move a1,s4
80100704:  lhu v1,0x42(s1)
80100708:  nop
8010070c:  subu v0,v0,v1
80100710:  sll v0,v0,0x14
80100714:  sra s2,v0,0x10
80100718:  slti v0,s2,-0x200
8010071c:  bne v0,zero,0x8010073c
80100720:  _li v1,-0x200
80100724:  li v1,0x200
80100728:  move v0,v1
8010072c:  slt v0,v0,s2
80100730:  bne v0,zero,0x8010073c
80100734:  _nop
80100738:  move v1,s2
8010073c:  lh v0,0x44(s1)
80100740:  nop
80100744:  subu v0,v1,v0
80100748:  sra v1,v0,0x4
8010074c:  slti v0,v1,-0x10
80100750:  bne v0,zero,0x80100770
80100754:  _li a0,-0x10
80100758:  li a0,0x10
8010075c:  move v0,a0
80100760:  slt v0,v0,v1
80100764:  bne v0,zero,0x80100770
80100768:  _nop
8010076c:  move a0,v1
80100770:  lhu v0,0x44(s1)
80100774:  lhu v1,0x42(s1)
80100778:  lui a1,0x1f
8010077c:  ori a1,a1,0x3fff
80100780:  addu v0,v0,a0
80100784:  sh v0,0x44(s1)
80100788:  sll v0,v0,0x10
8010078c:  sra v0,v0,0x16
80100790:  addu v1,v1,v0
80100794:  bgez s3,0x801007a0
80100798:  _move v0,s3
8010079c:  subu v0,zero,v0
801007a0:  slt v0,a1,v0
801007a4:  bne v0,zero,0x801008e8
801007a8:  _sh v1,0x42(s1)
801007ac:  bgez s4,0x801007b8
801007b0:  _move v0,s4
801007b4:  subu v0,zero,v0
801007b8:  slt v0,a1,v0
801007bc:  bne v0,zero,0x801008e8
801007c0:  _lui v0,0x8006
801007c4:  lw v0,0x5310(v0)
801007c8:  nop
801007cc:  andi v0,v0,0x1f
801007d0:  bne v0,zero,0x801008c0
801007d4:  _li a1,0xdb
801007d8:  lw a0,0x58(s1)
801007dc:  li a2,0x98
801007e0:  jal 0x8001ac44
801007e4:  _clear a3
801007e8:  move a0,v0
801007ec:  lw v0,0x0(a0)
801007f0:  li v1,0x7
801007f4:  sb v1,0x4(a0)
801007f8:  ori v0,v0,0x80
801007fc:  sw v0,0x0(a0)
80100800:  lhu v1,0x6(s1)
80100804:  lui v0,0x8010
80100808:  addiu v0,v0,0x59c
8010080c:  sw v0,0x64(a0)
80100810:  li v0,0x64
80100814:  sh v0,0xc(a0)
80100818:  sh v1,0x6(a0)
8010081c:  lw t0,0x48(s1)
80100820:  lw t1,0x4c(s1)
80100824:  lw t2,0x50(s1)
80100828:  sw t0,0x48(a0)
8010082c:  sw t1,0x4c(a0)
80100830:  sw t2,0x50(a0)
80100834:  lhu v0,0x42(s1)
80100838:  nop
8010083c:  sh v0,0x42(a0)
80100840:  lh v1,0x14(s1)
80100844:  nop
80100848:  sll v0,v1,0x2
8010084c:  addu v0,v0,v1
80100850:  sll v0,v0,0x5
80100854:  subu v0,v0,v1
80100858:  sll v0,v0,0x4
8010085c:  subu v0,v0,v1
80100860:  sll v1,v0,0x2
80100864:  subu v0,v1,v0
80100868:  bgez v0,0x80100874
8010086c:  _nop
80100870:  addiu v0,v0,0xfff
80100874:  sra v0,v0,0xc
80100878:  sw v0,0x88(a0)
8010087c:  lh v1,0x20(s1)
80100880:  nop
80100884:  sll v0,v1,0x2
80100888:  addu v0,v0,v1
8010088c:  sll v0,v0,0x5
80100890:  subu v0,v0,v1
80100894:  sll v0,v0,0x4
80100898:  subu v0,v0,v1
8010089c:  sll v1,v0,0x2
801008a0:  subu v0,v1,v0
801008a4:  bgez v0,0x801008b0
801008a8:  _nop
801008ac:  addiu v0,v0,0xfff
801008b0:  sra v0,v0,0xc
801008b4:  sw v0,0x90(a0)
801008b8:  jal 0x8002036c
801008bc:  _sw zero,0x8c(a0)
801008c0:  lw v0,0x4c(s0)
801008c4:  lw a0,0x4c(s1)
801008c8:  lui v1,0xfffd
801008cc:  ori v1,v1,0xd000
801008d0:  addu v0,v0,v1
801008d4:  slt v0,a0,v0
801008d8:  beq v0,zero,0x8010090c
801008dc:  _addiu v0,a0,0x5f5
801008e0:  slti a0,zero,0x243
801008e4:  sw v0,0x4c(s1)
801008e8:  lw v0,0x4c(s0)
801008ec:  lw a0,0x4c(s1)
801008f0:  lui v1,0xfff9
801008f4:  ori v1,v1,0xc000
801008f8:  addu v0,v0,v1
801008fc:  slt v0,v0,a0
80100900:  beq v0,zero,0x8010090c
80100904:  _addiu v0,a0,-0xbeb
80100908:  sw v0,0x4c(s1)
8010090c:  lhu v0,0xc(s0)
80100910:  nop
80100914:  beq v0,zero,0x80100934
80100918:  _nop
8010091c:  bgez s2,0x80100928
80100920:  _move v0,s2
80100924:  subu v0,zero,v0
80100928:  slti v0,v0,0x4001
8010092c:  bne v0,zero,0x801009ac
80100930:  _nop
80100934:  sb zero,0x8(s1)
80100938:  move a0,s1
8010093c:  jal 0x80020890
80100940:  _li a1,0x258
80100944:  slti a0,zero,0x26b
80100948:  nop
8010094c:  lh v0,0x44(s1)
80100950:  nop
80100954:  subu v0,zero,v0
80100958:  sra v1,v0,0x4
8010095c:  slti v0,v1,-0x10
80100960:  bne v0,zero,0x80100980
80100964:  _li a1,-0x10
80100968:  li a1,0x10
8010096c:  move v0,a1
80100970:  slt v0,v0,v1
80100974:  bne v0,zero,0x80100980
80100978:  _nop
8010097c:  move a1,v1
80100980:  lhu v0,0x44(s1)
80100984:  lhu v1,0x42(s1)
80100988:  lw a0,0x4c(s1)
8010098c:  addu v0,v0,a1
80100990:  sh v0,0x44(s1)
80100994:  sll v0,v0,0x10
80100998:  sra v0,v0,0x16
8010099c:  addu v1,v1,v0
801009a0:  addiu a0,a0,-0xbeb
801009a4:  sh v1,0x42(s1)
801009a8:  sw a0,0x4c(s1)
801009ac:  lh v0,0x14(s1)
801009b0:  nop
801009b4:  sll v1,v0,0x2
801009b8:  addu v1,v1,v0
801009bc:  sll v1,v1,0x5
801009c0:  subu v1,v1,v0
801009c4:  sll v1,v1,0x4
801009c8:  subu v1,v1,v0
801009cc:  sll v0,v1,0x2
801009d0:  subu a1,v0,v1
801009d4:  bgez a1,0x801009e0
801009d8:  _nop
801009dc:  addiu a1,a1,0xfff
801009e0:  lh v1,0x20(s1)
801009e4:  lw a0,0x48(s1)
801009e8:  sll v0,v1,0x2
801009ec:  addu v0,v0,v1
801009f0:  sll v0,v0,0x5
801009f4:  subu v0,v0,v1
801009f8:  sll v0,v0,0x4
801009fc:  subu v0,v0,v1
80100a00:  sll v1,v0,0x2
80100a04:  subu v1,v1,v0
80100a08:  sra v0,a1,0xc
80100a0c:  addu a0,a0,v0
80100a10:  bgez v1,0x80100a1c
80100a14:  _sw a0,0x48(s1)
80100a18:  addiu v1,v1,0xfff
80100a1c:  lw v0,0x50(s1)
80100a20:  move a0,s1
80100a24:  sra v1,v1,0xc
80100a28:  addu v0,v0,v1
80100a2c:  jal 0x8001d708
80100a30:  _sw v0,0x50(s1)
80100a34:  beq s5,zero,0x80100bf0
80100a38:  _nop
80100a3c:  lw s0,0x38(s1)
80100a40:  nop
80100a44:  beq s0,zero,0x80100a80
80100a48:  _sll s2,s5,0x8
80100a4c:  lh v0,0x6(s0)
80100a50:  nop
80100a54:  bne v0,zero,0x80100a70
80100a58:  _move a0,s0
80100a5c:  lhu v0,0x44(s0)
80100a60:  nop
80100a64:  addu v0,v0,s2
80100a68:  jal 0x8001d708
80100a6c:  _sh v0,0x44(s0)
80100a70:  lw s0,0x34(s0)
80100a74:  nop
80100a78:  bne s0,zero,0x80100a4c
80100a7c:  _nop
80100a80:  jal 0x800449bc
80100a84:  _addiu a0,s1,0x48
80100a88:  lb a0,0x5(s1)
80100a8c:  jal 0x80044574
80100a90:  _sll a1,v0,0x1
80100a94:  slti a0,zero,0x2fc
80100a98:  nop
80100a9c:  lw v0,0x0(s1)
80100aa0:  nop
80100aa4:  andi v0,v0,0x80
80100aa8:  beq v0,zero,0x80100b00
80100aac:  _lui v0,0x8006
80100ab0:  jal 0x80020778
80100ab4:  _move a0,s1
80100ab8:  lb a0,0x5(s1)
80100abc:  jal 0x800441c8
80100ac0:  _nop
80100ac4:  lw a0,0x48(s1)
80100ac8:  lw a1,0x50(s1)
80100acc:  lw v0,0x0(s1)
80100ad0:  lhu v1,0x42(s1)
80100ad4:  sb zero,0x5(s1)
80100ad8:  ori v0,v0,0x22
80100adc:  xori v1,v1,0x800
80100ae0:  sw v0,0x0(s1)
80100ae4:  jal 0x80025400
80100ae8:  _sh v1,0x42(s1)
80100aec:  lui v1,0xfff9
80100af0:  ori v1,v1,0xc000
80100af4:  addu v0,v0,v1
80100af8:  sw v0,0x4c(s1)
80100afc:  lui v0,0x8006
80100b00:  lw s0,0x5ad4(v0)
80100b04:  nop
80100b08:  lw a0,0x24(s0)
80100b0c:  lw a1,0x2c(s0)
80100b10:  jal 0x8001fd08
80100b14:  _nop
80100b18:  beq v0,zero,0x80100b30
80100b1c:  _nop
80100b20:  lh v0,0xa(v0)
80100b24:  nop
80100b28:  beq v0,zero,0x80100b6c
80100b2c:  _nop
80100b30:  lui v0,0x8006
80100b34:  lw s0,0x5ad8(v0)
80100b38:  nop
80100b3c:  beq s0,zero,0x80100bdc
80100b40:  _nop
80100b44:  lw a0,0x24(s0)
80100b48:  lw a1,0x2c(s0)
80100b4c:  jal 0x8001fd08
80100b50:  _nop
80100b54:  beq v0,zero,0x80100bdc
80100b58:  _nop
80100b5c:  lh v0,0xa(v0)
80100b60:  nop
80100b64:  bne v0,zero,0x80100bdc
80100b68:  _nop
80100b6c:  lbu v0,0x8(s1)
80100b70:  li v1,0x1
80100b74:  addiu v0,v0,0x1
80100b78:  sb v0,0x8(s1)
80100b7c:  sll v0,v0,0x18
80100b80:  sra v0,v0,0x18
80100b84:  beq v0,v1,0x80100bd0
80100b88:  _li v1,-0x23
80100b8c:  lw v0,0x0(s1)
80100b90:  sw s0,0xa4(s1)
80100b94:  and v0,v0,v1
80100b98:  jal 0x8004410c
80100b9c:  _sw v0,0x0(s1)
80100ba0:  lw v1,0x58(s1)
80100ba4:  sb v0,0x5(s1)
80100ba8:  lw a1,0x8(v1)
80100bac:  sll v0,v0,0x18
80100bb0:  sra a0,v0,0x18
80100bb4:  li a2,0x1
80100bb8:  jal 0x800443c8
80100bbc:  _clear a3
80100bc0:  jal 0x80020744
80100bc4:  _move a0,s1
80100bc8:  slti a0,zero,0x2fc
80100bcc:  nop
80100bd0:  move a0,s1
80100bd4:  slti a0,zero,0x2fa
80100bd8:  li a1,0x78
80100bdc:  sb zero,0x8(s1)
80100be0:  move a0,s1
80100be4:  li a1,0x3c
80100be8:  jal 0x80020890
80100bec:  _nop
80100bf0:  lw ra,0x28(sp)
80100bf4:  lw s5,0x24(sp)
80100bf8:  lw s4,0x20(sp)
80100bfc:  lw s3,0x1c(sp)
80100c00:  lw s2,0x18(sp)
80100c04:  lw s1,0x14(sp)
80100c08:  lw s0,0x10(sp)
80100c0c:  clear v0
80100c10:  jr ra
80100c14:  _addiu sp,sp,0x30
