# addr: 0x801006f0  name: FUN_801006f0
801006f0:  addiu sp,sp,-0x98
801006f4:  sw s0,0x70(sp)
801006f8:  move s0,a1
801006fc:  sw a0,0x98(sp)
80100700:  addiu a0,sp,0x98
80100704:  sw ra,0x94(sp)
80100708:  sw s8,0x90(sp)
8010070c:  sw s7,0x8c(sp)
80100710:  sw s6,0x88(sp)
80100714:  sw s5,0x84(sp)
80100718:  sw s4,0x80(sp)
8010071c:  sw s3,0x7c(sp)
80100720:  sw s2,0x78(sp)
80100724:  jal 0x8002249c
80100728:  _sw s1,0x74(sp)
8010072c:  addiu a0,sp,0x98
80100730:  jal 0x8002249c
80100734:  _sw v0,0x68(sp)
80100738:  addiu a0,sp,0x98
8010073c:  jal 0x800224b4
80100740:  _move s6,v0
80100744:  addiu a0,sp,0x98
80100748:  jal 0x800224ec
8010074c:  _move s8,v0
80100750:  lui v1,0xfff8
80100754:  ori v1,v1,0x67fe
80100758:  addiu a0,sp,0x98
8010075c:  jal 0x800224ec
80100760:  _and s4,v0,v1
80100764:  addiu a0,sp,0x98
80100768:  jal 0x800224ec
8010076c:  _sw v0,0x10(sp)
80100770:  addiu a0,sp,0x98
80100774:  lui v1,0xfff0
80100778:  addu v0,v0,v1
8010077c:  jal 0x800224ec
80100780:  _sw v0,0x14(sp)
80100784:  addiu a0,sp,0x98
80100788:  jal 0x800224b4
8010078c:  _sw v0,0x18(sp)
80100790:  addiu a0,sp,0x98
80100794:  jal 0x800224b4
80100798:  _sh v0,0x20(sp)
8010079c:  addiu a0,sp,0x98
801007a0:  jal 0x800224b4
801007a4:  _sh v0,0x22(sp)
801007a8:  addiu a0,sp,0x98
801007ac:  jal 0x800224b4
801007b0:  _sh v0,0x24(sp)
801007b4:  addiu a0,sp,0x98
801007b8:  sll v0,v0,0x10
801007bc:  sra v0,v0,0x10
801007c0:  jal 0x800224b4
801007c4:  _addiu s5,v0,0x12
801007c8:  addiu a0,sp,0x98
801007cc:  jal 0x800224ec
801007d0:  _move s7,v0
801007d4:  lw a1,0x98(sp)
801007d8:  addiu s1,sp,0x28
801007dc:  move a0,s1
801007e0:  addiu s0,s0,-0x22
801007e4:  move a2,s0
801007e8:  jal 0x80044c44
801007ec:  _move s2,v0
801007f0:  clear a0
801007f4:  move a1,s1
801007f8:  addu s0,s1,s0
801007fc:  jal 0x80011aa8
80100800:  _sb zero,0x0(s0)
80100804:  bne v0,zero,0x80100834
80100808:  _move s3,v0
8010080c:  lui v0,0x8006
80100810:  lw v0,0x5a38(v0)
80100814:  nop
80100818:  lw a0,0x4(v0)
8010081c:  jal 0x80011a38
80100820:  _move a1,s1
80100824:  bne v0,zero,0x80100834
80100828:  _move s3,v0
8010082c:  lui v0,0x8002
80100830:  addiu s3,v0,0x23dc
80100834:  andi v0,s4,0x4
80100838:  beq v0,zero,0x80100888
8010083c:  _sltiu v0,s6,0x7
80100840:  jal 0x80017160
80100844:  _nop
80100848:  lui a0,0x8007
8010084c:  addiu a0,a0,0x37a0
80100850:  sll v1,s5,0x2
80100854:  addu v1,v1,a0
80100858:  lw v1,0x0(v1)
8010085c:  nop
80100860:  lw v1,0x4(v1)
80100864:  nop
80100868:  lw v1,0x0(v1)
8010086c:  nop
80100870:  mult v0,v1
80100874:  lui v1,0x8006
80100878:  mflo t0
8010087c:  sra v0,t0,0xf
80100880:  sh v0,0x59d0(v1)
80100884:  sltiu v0,s6,0x7
80100888:  beq v0,zero,0x80100cec
8010088c:  _sll v1,s6,0x2
80100890:  lui v0,0x8010
80100894:  addiu v0,v0,0x8
80100898:  addu v1,v1,v0
8010089c:  lw v0,0x0(v1)
801008a0:  nop
801008a4:  jr v0
801008a8:  _nop
801008ac:  lui v1,0x8007
801008b0:  addiu v1,v1,0x37a0
801008b4:  sll v0,s5,0x2
801008b8:  addu v0,v0,v1
801008bc:  lw a1,0x0(v0)
801008c0:  move a0,s3
801008c4:  andi a2,s7,0xffff
801008c8:  sll a3,s4,0x1
801008cc:  jal 0x80021b80
801008d0:  _andi a3,a3,0x8
801008d4:  move s0,v0
801008d8:  sw s4,0x0(s0)
801008dc:  sb s6,0x4(s0)
801008e0:  sh s8,0x6(s0)
801008e4:  lbu t1,0x68(sp)
801008e8:  nop
801008ec:  sb t1,0x8(s0)
801008f0:  lw t2,0x10(sp)
801008f4:  lw t3,0x14(sp)
801008f8:  lw t0,0x18(sp)
801008fc:  sw t2,0x48(s0)
80100900:  sw t3,0x4c(s0)
80100904:  sw t0,0x50(s0)
80100908:  lwl t2,0x23(sp)
8010090c:  lwr t2,0x20(sp)
80100910:  lh t3,0x24(sp)
80100914:  swl t2,0x43(s0)
80100918:  swr t2,0x40(s0)
8010091c:  sh t3,0x44(s0)
80100920:  sh s2,0xe(s0)
80100924:  jal 0x80017160
80100928:  _sh s2,0xc(s0)
8010092c:  move a0,s0
80100930:  sb v0,0x9(s0)
80100934:  jal 0x8001d708
80100938:  _sw s3,0x64(s0)
8010093c:  jal 0x8001dc1c
80100940:  _move a0,s0
80100944:  lw v0,0x64(s0)
80100948:  nop
8010094c:  beq v0,zero,0x80100968
80100950:  _move a0,s0
80100954:  li a1,0x1
80100958:  jalr v0
8010095c:  _clear a2
80100960:  slti a0,zero,0x25b
80100964:  nop
80100968:  clear v0
8010096c:  bltz v0,0x80100cec
80100970:  _clear v0
80100974:  jal 0x8001b0c4
80100978:  _move a0,s0
8010097c:  lw v0,0x0(s0)
80100980:  nop
80100984:  andi v0,v0,0x8
80100988:  beq v0,zero,0x801009a8
8010098c:  _nop
80100990:  lw v0,0x70(s0)
80100994:  nop
80100998:  bne v0,zero,0x801009a8
8010099c:  _nop
801009a0:  jal 0x8003e730
801009a4:  _move a0,s0
801009a8:  lw v0,0x0(s0)
801009ac:  nop
801009b0:  andi v0,v0,0x4
801009b4:  beq v0,zero,0x801009c8
801009b8:  _lui a0,0x8006
801009bc:  addiu a0,a0,0x5a80
801009c0:  jal 0x8001fe50
801009c4:  _move a1,s0
801009c8:  lw v0,0x0(s0)
801009cc:  nop
801009d0:  andi v0,v0,0x80
801009d4:  beq v0,zero,0x801009e8
801009d8:  _lui a0,0x8006
801009dc:  addiu a0,a0,0x5a60
801009e0:  jal 0x8001fe50
801009e4:  _move a1,s0
801009e8:  jal 0x8001ec48
801009ec:  _move a0,s0
801009f0:  lui a0,0x8010
801009f4:  addiu a0,a0,0x7da0
801009f8:  jal 0x8001fe50
801009fc:  _move a1,s0
80100a00:  slti a0,zero,0x33b
80100a04:  move v0,s0
80100a08:  lui v1,0x8007
80100a0c:  addiu v1,v1,0x37a0
80100a10:  sll v0,s5,0x2
80100a14:  addu v0,v0,v1
80100a18:  lw a1,0x0(v0)
80100a1c:  move a0,s3
80100a20:  andi a2,s7,0xffff
80100a24:  sll a3,s4,0x1
80100a28:  jal 0x80021b80
80100a2c:  _andi a3,a3,0x8
80100a30:  move s0,v0
80100a34:  sw s4,0x0(s0)
80100a38:  sb s6,0x4(s0)
80100a3c:  sh s8,0x6(s0)
80100a40:  lbu t2,0x68(sp)
80100a44:  nop
80100a48:  sb t2,0x8(s0)
80100a4c:  lw t3,0x10(sp)
80100a50:  lw t0,0x14(sp)
80100a54:  lw t1,0x18(sp)
80100a58:  sw t3,0x48(s0)
80100a5c:  sw t0,0x4c(s0)
80100a60:  sw t1,0x50(s0)
80100a64:  lwl t3,0x23(sp)
80100a68:  lwr t3,0x20(sp)
80100a6c:  lh t0,0x24(sp)
80100a70:  swl t3,0x43(s0)
80100a74:  swr t3,0x40(s0)
80100a78:  sh t0,0x44(s0)
80100a7c:  lw v0,0x38(s0)
80100a80:  sh s2,0xe(s0)
80100a84:  beq v0,zero,0x80100aa4
80100a88:  _sh s2,0xc(s0)
80100a8c:  sh s2,0xe(v0)
80100a90:  sh s2,0xc(v0)
80100a94:  lw v0,0x34(v0)
80100a98:  nop
80100a9c:  bne v0,zero,0x80100a8c
80100aa0:  _nop
80100aa4:  jal 0x80017160
80100aa8:  _nop
80100aac:  move a0,s0
80100ab0:  sb v0,0x9(s0)
80100ab4:  jal 0x8002036c
80100ab8:  _sw s3,0x64(s0)
80100abc:  beq v0,zero,0x80100cec
80100ac0:  _clear v0
80100ac4:  jal 0x8001b0c4
80100ac8:  _move a0,s0
80100acc:  slti a0,zero,0x33b
80100ad0:  move v0,s0
80100ad4:  jal 0x8001d470
80100ad8:  _li a0,0x80
80100adc:  move s1,v0
80100ae0:  lui v1,0x8007
80100ae4:  addiu v1,v1,0x37a0
80100ae8:  sll v0,s5,0x2
80100aec:  addu v0,v0,v1
80100af0:  sw s3,0x64(s1)
80100af4:  lw v0,0x0(v0)
80100af8:  sh s7,0xa(s1)
80100afc:  sw s4,0x0(s1)
80100b00:  sb s6,0x4(s1)
80100b04:  sh s8,0x6(s1)
80100b08:  lbu t3,0x68(sp)
80100b0c:  nop
80100b10:  sb t3,0x8(s1)
80100b14:  sw v0,0x58(s1)
80100b18:  lw t0,0x10(sp)
80100b1c:  lw t1,0x14(sp)
80100b20:  lw t2,0x18(sp)
80100b24:  sw t0,0x48(s1)
80100b28:  sw t1,0x4c(s1)
80100b2c:  sw t2,0x50(s1)
80100b30:  lwl t0,0x23(sp)
80100b34:  lwr t0,0x20(sp)
80100b38:  lh t1,0x24(sp)
80100b3c:  swl t0,0x43(s1)
80100b40:  swr t0,0x40(s1)
80100b44:  sh t1,0x44(s1)
80100b48:  lh v0,0x6(s1)
80100b4c:  nop
80100b50:  bltz v0,0x80100b80
80100b54:  _lui v0,0x8006
80100b58:  lb v0,0x531a(v0)
80100b5c:  nop
80100b60:  addiu v0,v0,0x2
80100b64:  mult s2,v0
80100b68:  mflo v0
80100b6c:  bgez v0,0x80100b78
80100b70:  _nop
80100b74:  addiu v0,v0,0x3
80100b78:  slti a0,zero,0x2e1
80100b7c:  srl v0,v0,0x2
80100b80:  move v0,s2
80100b84:  sh v0,0xe(s1)
80100b88:  jal 0x80017160
80100b8c:  _sh v0,0xc(s1)
80100b90:  move a0,s1
80100b94:  jal 0x8001d708
80100b98:  _sb v0,0x9(s1)
80100b9c:  lui v0,0x8006
80100ba0:  lw s0,0x5a50(v0)
80100ba4:  nop
80100ba8:  lw v1,0x0(s0)
80100bac:  nop
80100bb0:  beq v1,zero,0x80100c48
80100bb4:  _nop
80100bb8:  sll v0,s8,0x10
80100bbc:  sra a0,v0,0x10
80100bc0:  lw v0,0x8(s0)
80100bc4:  nop
80100bc8:  lh v0,0x6(v0)
80100bcc:  nop
80100bd0:  slt v0,v0,a0
80100bd4:  beq v0,zero,0x80100bf0
80100bd8:  _nop
80100bdc:  move s0,v1
80100be0:  lw v1,0x0(s0)
80100be4:  nop
80100be8:  bne v1,zero,0x80100bc0
80100bec:  _nop
80100bf0:  beq v1,zero,0x80100c48
80100bf4:  _nop
80100bf8:  lw a0,0x8(s0)
80100bfc:  nop
80100c00:  lh v1,0x6(a0)
80100c04:  sll v0,s8,0x10
80100c08:  sra v0,v0,0x10
80100c0c:  bne v0,v1,0x80100c48
80100c10:  _move v1,a0
80100c14:  lw v0,0x34(v1)
80100c18:  nop
80100c1c:  beq v0,zero,0x80100c3c
80100c20:  _nop
80100c24:  lw v1,0x34(v1)
80100c28:  nop
80100c2c:  lw v0,0x34(v1)
80100c30:  nop
80100c34:  bne v0,zero,0x80100c24
80100c38:  _nop
80100c3c:  sw s1,0x34(v1)
80100c40:  slti a0,zero,0x31a
80100c44:  sw v1,0x3c(s1)
80100c48:  jal 0x80022c54
80100c4c:  _move a0,s1
80100c50:  lw v1,0x4(s0)
80100c54:  nop
80100c58:  sw v0,0x0(v1)
80100c5c:  sw v0,0x4(s0)
80100c60:  sw s0,0x0(v0)
80100c64:  sw v1,0x4(v0)
80100c68:  slti a0,zero,0x33b
80100c6c:  move v0,s1
80100c70:  jal 0x8001d470
80100c74:  _li a0,0x94
80100c78:  move s0,v0
80100c7c:  sw s4,0x0(s0)
80100c80:  sb s6,0x4(s0)
80100c84:  sh s8,0x6(s0)
80100c88:  lbu t0,0x68(sp)
80100c8c:  nop
80100c90:  sb t0,0x8(s0)
80100c94:  lw t1,0x10(sp)
80100c98:  lw t2,0x14(sp)
80100c9c:  lw t3,0x18(sp)
80100ca0:  sw t1,0x48(s0)
80100ca4:  sw t2,0x4c(s0)
80100ca8:  sw t3,0x50(s0)
80100cac:  lwl t1,0x23(sp)
80100cb0:  lwr t1,0x20(sp)
80100cb4:  lh t2,0x24(sp)
80100cb8:  swl t1,0x43(s0)
80100cbc:  swr t1,0x40(s0)
80100cc0:  sh t2,0x44(s0)
80100cc4:  jal 0x80017160
80100cc8:  _nop
80100ccc:  move a0,s0
80100cd0:  jal 0x8001d708
80100cd4:  _sb v0,0x9(s0)
80100cd8:  lui a0,0x8010
80100cdc:  addiu a0,a0,0x7d90
80100ce0:  jal 0x8001fe50
80100ce4:  _move a1,s0
80100ce8:  move v0,s0
80100cec:  lw ra,0x94(sp)
80100cf0:  lw s8,0x90(sp)
80100cf4:  lw s7,0x8c(sp)
80100cf8:  lw s6,0x88(sp)
80100cfc:  lw s5,0x84(sp)
80100d00:  lw s4,0x80(sp)
80100d04:  lw s3,0x7c(sp)
80100d08:  lw s2,0x78(sp)
80100d0c:  lw s1,0x74(sp)
80100d10:  lw s0,0x70(sp)
80100d14:  jr ra
80100d18:  _addiu sp,sp,0x98
