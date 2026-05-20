# addr: 0x80100970  name: FUN_80100970
80100970:  addiu sp,sp,-0x48
80100974:  sw s3,0x34(sp)
80100978:  move s3,a0
8010097c:  move v1,a1
80100980:  move a1,a2
80100984:  sltiu v0,v1,0x9
80100988:  sw ra,0x40(sp)
8010098c:  sw s5,0x3c(sp)
80100990:  sw s4,0x38(sp)
80100994:  sw s2,0x30(sp)
80100998:  sw s1,0x2c(sp)
8010099c:  beq v0,zero,0x80100c78
801009a0:  _sw s0,0x28(sp)
801009a4:  lui v0,0x8010
801009a8:  addiu v0,v0,0xc8
801009ac:  sll v1,v1,0x2
801009b0:  addu v1,v1,v0
801009b4:  lw v0,0x0(v1)
801009b8:  nop
801009bc:  jr v0
801009c0:  _nop
801009c4:  lw s1,0x38(s3)
801009c8:  lw s2,0x78(s3)
801009cc:  jal 0x8001d624
801009d0:  _move a0,s1
801009d4:  move a0,v0
801009d8:  addiu a1,s2,0x48
801009dc:  jal 0x800435c0
801009e0:  _addiu a2,sp,0x18
801009e4:  lw a0,0x18(sp)
801009e8:  lw a1,0x20(sp)
801009ec:  jal 0x8004ecd4
801009f0:  _nop
801009f4:  lw a0,0x1c(sp)
801009f8:  lw a1,0x20(sp)
801009fc:  sll s0,v0,0x14
80100a00:  jal 0x8004ecd4
80100a04:  _sra s0,s0,0x14
80100a08:  subu v0,zero,v0
80100a0c:  sll v0,v0,0x14
80100a10:  bgez s0,0x80100a1c
80100a14:  _sra v1,v0,0x14
80100a18:  addiu s0,s0,0x3
80100a1c:  lhu v0,0x42(s1)
80100a20:  move a0,v1
80100a24:  sra v1,s0,0x2
80100a28:  addu v0,v0,v1
80100a2c:  sh v0,0x42(s1)
80100a30:  lh v1,0x40(s1)
80100a34:  bgez a0,0x80100a44
80100a38:  _sra v0,a0,0x2
80100a3c:  addiu a0,a0,0x3
80100a40:  sra v0,a0,0x2
80100a44:  addu v1,v1,v0
80100a48:  slti v0,v1,-0x155
80100a4c:  bne v0,zero,0x80100a70
80100a50:  _li a1,-0x155
80100a54:  li a1,0x155
80100a58:  move v0,a1
80100a5c:  slt v0,v0,v1
80100a60:  bne v0,zero,0x80100a74
80100a64:  _move a0,s1
80100a68:  slti a0,zero,0x29d
80100a6c:  move a1,v1
80100a70:  move a0,s1
80100a74:  jal 0x8001d708
80100a78:  _sh a1,0x40(a0)
80100a7c:  lw v1,0x20(sp)
80100a80:  lui v0,0xf
80100a84:  ori v0,v0,0xa000
80100a88:  slt v0,v0,v1
80100a8c:  bne v0,zero,0x80100aa4
80100a90:  _nop
80100a94:  lhu v0,0xc(s2)
80100a98:  nop
80100a9c:  bne v0,zero,0x80100c78
80100aa0:  _nop
80100aa4:  slti a0,zero,0x315
80100aa8:  sw zero,0x78(s3)
80100aac:  lw v0,0x78(s3)
80100ab0:  nop
80100ab4:  beq v0,zero,0x80100ba0
80100ab8:  _li a1,0x1e3
80100abc:  lw a0,0x58(s3)
80100ac0:  li a2,0x80
80100ac4:  jal 0x8001ac44
80100ac8:  _li a3,0x8
80100acc:  move s1,v0
80100ad0:  sw s1,0x10(sp)
80100ad4:  lw a1,0x38(s3)
80100ad8:  move a0,s3
80100adc:  li a2,0x1e1
80100ae0:  jal 0x80031300
80100ae4:  _li a3,0x98
80100ae8:  move s0,v0
80100aec:  move a0,s0
80100af0:  li v0,0x84
80100af4:  sw v0,0x0(s0)
80100af8:  li v0,0x32
80100afc:  sh v0,0xc(s0)
80100b00:  lui v0,0x8010
80100b04:  addiu v0,v0,0x76c
80100b08:  sw v0,0x64(s0)
80100b0c:  li v0,0x4
80100b10:  sh v0,0x96(s0)
80100b14:  li v0,0x8
80100b18:  jal 0x800202f4
80100b1c:  _sh v0,0x94(s0)
80100b20:  move a0,s1
80100b24:  li v0,0x10
80100b28:  sw v0,0x0(a0)
80100b2c:  lui v0,0x8004
80100b30:  addiu v0,v0,-0x17f4
80100b34:  jal 0x800207c4
80100b38:  _sw v0,0x64(a0)
80100b3c:  jal 0x8004410c
80100b40:  _nop
80100b44:  lw v1,0x58(s3)
80100b48:  nop
80100b4c:  lw a1,0x8(v1)
80100b50:  move a0,v0
80100b54:  clear a2
80100b58:  jal 0x8004483c
80100b5c:  _addiu a3,s0,0x24
80100b60:  slti a0,zero,0x306
80100b64:  move a0,s3
80100b68:  move a0,s3
80100b6c:  jal 0x80020744
80100b70:  _sw s0,0x78(s3)
80100b74:  jal 0x8004410c
80100b78:  _nop
80100b7c:  lw v1,0x58(s3)
80100b80:  nop
80100b84:  lw a1,0x8(v1)
80100b88:  move a0,v0
80100b8c:  li a2,0x2
80100b90:  jal 0x800447e8
80100b94:  _move a3,s2
80100b98:  slti a0,zero,0x306
80100b9c:  move a0,s3
80100ba0:  lui v0,0x8006
80100ba4:  lw a1,0x5a18(v0)
80100ba8:  nop
80100bac:  lw s1,0x0(a1)
80100bb0:  nop
80100bb4:  beq s1,zero,0x80100c14
80100bb8:  _li s5,0x2
80100bbc:  addiu s2,s3,0x48
80100bc0:  lui s4,0xc
80100bc4:  ori s4,s4,0x7fff
80100bc8:  lw s0,0x8(a1)
80100bcc:  nop
80100bd0:  lbu v0,0x4(s0)
80100bd4:  nop
80100bd8:  bne v0,s5,0x80100c04
80100bdc:  _move a1,s1
80100be0:  lhu v0,0xc(s0)
80100be4:  nop
80100be8:  beq v0,zero,0x80100c04
80100bec:  _move a0,s2
80100bf0:  jal 0x80016aac
80100bf4:  _addiu a1,s0,0x48
80100bf8:  slt v0,s4,v0
80100bfc:  beq v0,zero,0x80100b68
80100c00:  _move a1,s1
80100c04:  lw s1,0x0(s1)
80100c08:  nop
80100c0c:  bne s1,zero,0x80100bc8
80100c10:  _nop
80100c14:  move a0,s3
80100c18:  slti a0,zero,0x31c
80100c1c:  li a1,0x3c
80100c20:  lw a1,0x0(a1)
80100c24:  nop
80100c28:  lbu v1,0x4(a1)
80100c2c:  li v0,0x7
80100c30:  bne v1,v0,0x80100c78
80100c34:  _nop
80100c38:  lhu a1,0xc(a1)
80100c3c:  jal 0x80022320
80100c40:  _move a0,s3
80100c44:  beq v0,zero,0x80100c78
80100c48:  _nop
80100c4c:  jal 0x80020844
80100c50:  _move a0,s3
80100c54:  jal 0x80020778
80100c58:  _move a0,s3
80100c5c:  slti a0,zero,0x31e
80100c60:  nop
80100c64:  lbu a1,0x9(s3)
80100c68:  move a0,s3
80100c6c:  addiu a1,a1,0x1e
80100c70:  jal 0x80020890
80100c74:  _nop
80100c78:  lw ra,0x40(sp)
80100c7c:  lw s5,0x3c(sp)
80100c80:  lw s4,0x38(sp)
80100c84:  lw s3,0x34(sp)
80100c88:  lw s2,0x30(sp)
80100c8c:  lw s1,0x2c(sp)
80100c90:  lw s0,0x28(sp)
80100c94:  clear v0
80100c98:  jr ra
80100c9c:  _addiu sp,sp,0x48
