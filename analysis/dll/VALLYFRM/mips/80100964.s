# addr: 0x80100964  name: FUN_80100964
80100964:  addiu sp,sp,-0x38
80100968:  sw s6,0x28(sp)
8010096c:  move s6,a0
80100970:  li v0,0x1
80100974:  sw ra,0x30(sp)
80100978:  sw s7,0x2c(sp)
8010097c:  sw s5,0x24(sp)
80100980:  sw s4,0x20(sp)
80100984:  sw s3,0x1c(sp)
80100988:  sw s2,0x18(sp)
8010098c:  sw s1,0x14(sp)
80100990:  beq a1,v0,0x80100bd8
80100994:  _sw s0,0x10(sp)
80100998:  beq a1,zero,0x801009b8
8010099c:  _li v0,0x3
801009a0:  beq a1,v0,0x801009e8
801009a4:  _li v0,0x8
801009a8:  beq a1,v0,0x80100a04
801009ac:  _move a0,s6
801009b0:  slti a0,zero,0x2fb
801009b4:  nop
801009b8:  lw v1,0x38(s6)
801009bc:  nop
801009c0:  lhu v0,0x44(v1)
801009c4:  nop
801009c8:  addiu v0,v0,0x44
801009cc:  beq a2,zero,0x80100bec
801009d0:  _sh v0,0x44(v1)
801009d4:  lw a0,0x38(s6)
801009d8:  jal 0x8001d708
801009dc:  _nop
801009e0:  slti a0,zero,0x2fb
801009e4:  nop
801009e8:  move a0,s6
801009ec:  jal 0x8002239c
801009f0:  _move a1,a2
801009f4:  beq v0,zero,0x80100bec
801009f8:  _nop
801009fc:  slti a0,zero,0x285
80100a00:  nop
80100a04:  jal 0x80022320
80100a08:  _move a1,a2
80100a0c:  beq v0,zero,0x80100bec
80100a10:  _nop
80100a14:  lw a0,0x38(s6)
80100a18:  jal 0x8001d624
80100a1c:  _clear s5
80100a20:  lui v1,0x8006
80100a24:  lw a1,0x5a18(v1)
80100a28:  nop
80100a2c:  lw s1,0x0(a1)
80100a30:  move s4,s5
80100a34:  li s3,-0x1
80100a38:  move s2,s3
80100a3c:  beq s1,zero,0x80100ac4
80100a40:  _move s7,v0
80100a44:  lw s0,0x8(a1)
80100a48:  nop
80100a4c:  lbu v1,0x4(s0)
80100a50:  li v0,0x2
80100a54:  bne v1,v0,0x80100ab4
80100a58:  _move a1,s1
80100a5c:  lhu v0,0xc(s0)
80100a60:  nop
80100a64:  beq v0,zero,0x80100ab4
80100a68:  _addiu a0,s6,0x48
80100a6c:  jal 0x80016aac
80100a70:  _addiu a1,s0,0x48
80100a74:  lh v1,0x6(s0)
80100a78:  nop
80100a7c:  blez v1,0x80100a9c
80100a80:  _move a0,v0
80100a84:  sltu v0,a0,s2
80100a88:  beq v0,zero,0x80100aa0
80100a8c:  _sltu v0,a0,s3
80100a90:  move s4,s0
80100a94:  slti a0,zero,0x2ac
80100a98:  move s2,a0
80100a9c:  sltu v0,a0,s3
80100aa0:  beq v0,zero,0x80100ab4
80100aa4:  _move a1,s1
80100aa8:  move s5,s0
80100aac:  move s3,a0
80100ab0:  move a1,s1
80100ab4:  lw s1,0x0(s1)
80100ab8:  nop
80100abc:  bne s1,zero,0x80100a44
80100ac0:  _nop
80100ac4:  bne s4,zero,0x80100ad0
80100ac8:  _nop
80100acc:  move s4,s5
80100ad0:  lw a0,0x38(s6)
80100ad4:  jal 0x8001d564
80100ad8:  _nop
80100adc:  move a0,v0
80100ae0:  jal 0x80045134
80100ae4:  _li a1,0x98
80100ae8:  move s0,v0
80100aec:  sw s4,0x84(s0)
80100af0:  lhu v0,0x6(s6)
80100af4:  move a0,s0
80100af8:  sh v0,0x6(s0)
80100afc:  li v0,0x7
80100b00:  lw t0,0x0(s7)
80100b04:  lw t1,0x4(s7)
80100b08:  lw t2,0x8(s7)
80100b0c:  lw t3,0xc(s7)
80100b10:  sw t0,0x10(s0)
80100b14:  sw t1,0x14(s0)
80100b18:  sw t2,0x18(s0)
80100b1c:  sw t3,0x1c(s0)
80100b20:  lw t0,0x10(s7)
80100b24:  lw t1,0x14(s7)
80100b28:  lw t2,0x18(s7)
80100b2c:  lw t3,0x1c(s7)
80100b30:  sw t0,0x20(s0)
80100b34:  sw t1,0x24(s0)
80100b38:  sw t2,0x28(s0)
80100b3c:  sw t3,0x2c(s0)
80100b40:  lw t0,0x14(s7)
80100b44:  lw t1,0x18(s7)
80100b48:  lw t2,0x1c(s7)
80100b4c:  sw t0,0x48(s0)
80100b50:  sw t1,0x4c(s0)
80100b54:  sw t2,0x50(s0)
80100b58:  sb v0,0x4(s0)
80100b5c:  li v0,0x80
80100b60:  sw v0,0x0(s0)
80100b64:  li v0,0x64
80100b68:  sh v0,0xc(s0)
80100b6c:  lui v0,0x8010
80100b70:  addiu v0,v0,0x5e8
80100b74:  jal 0x800202f4
80100b78:  _sw v0,0x64(s0)
80100b7c:  lh v0,0x14(s0)
80100b80:  lh v1,0x1a(s0)
80100b84:  addiu a0,s0,0x88
80100b88:  sll v0,v0,0x1
80100b8c:  sll v1,v1,0x1
80100b90:  sw v0,0x88(s0)
80100b94:  sw v1,0x4(a0)
80100b98:  lh v0,0x20(s0)
80100b9c:  nop
80100ba0:  sll v0,v0,0x1
80100ba4:  jal 0x8004410c
80100ba8:  _sw v0,0x8(a0)
80100bac:  sb v0,0x5(s0)
80100bb0:  lw v1,0x58(s6)
80100bb4:  nop
80100bb8:  lw a1,0x8(v1)
80100bbc:  sll v0,v0,0x18
80100bc0:  sra a0,v0,0x18
80100bc4:  li a2,0x1
80100bc8:  jal 0x8004483c
80100bcc:  _addiu a3,s0,0x24
80100bd0:  slti a0,zero,0x2fb
80100bd4:  nop
80100bd8:  jal 0x80017160
80100bdc:  _nop
80100be0:  lw v1,0x38(s6)
80100be4:  nop
80100be8:  sh v0,0x44(v1)
80100bec:  lw ra,0x30(sp)
80100bf0:  lw s7,0x2c(sp)
80100bf4:  lw s6,0x28(sp)
80100bf8:  lw s5,0x24(sp)
80100bfc:  lw s4,0x20(sp)
80100c00:  lw s3,0x1c(sp)
80100c04:  lw s2,0x18(sp)
80100c08:  lw s1,0x14(sp)
80100c0c:  lw s0,0x10(sp)
80100c10:  clear v0
80100c14:  jr ra
80100c18:  _addiu sp,sp,0x38
