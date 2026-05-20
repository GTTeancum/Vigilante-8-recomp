# addr: 0x80054984  name: FUN_80054984
80054984:  addiu sp,sp,-0x28
80054988:  sw s0,0x10(sp)
8005498c:  move s0,a0
80054990:  sw s3,0x1c(sp)
80054994:  move s3,a1
80054998:  sw ra,0x20(sp)
8005499c:  sw s2,0x18(sp)
800549a0:  sw s1,0x14(sp)
800549a4:  lw v0,0x3c(s0)
800549a8:  nop
800549ac:  lbu v0,0x0(v0)
800549b0:  li v1,0x8
800549b4:  sra v0,v0,0x4
800549b8:  bne v0,v1,0x800549d8
800549bc:  _li s1,0x88
800549c0:  lbu v0,0x44(s0)
800549c4:  nop
800549c8:  sltiu v0,v0,0x9
800549cc:  bne v0,zero,0x800549d8
800549d0:  _nop
800549d4:  li s1,0x22
800549d8:  lui v1,0x8006
800549dc:  lw v1,0x52c0(v1)
800549e0:  nop
800549e4:  lhu v0,0x4(v1)
800549e8:  nop
800549ec:  andi v0,v0,0x2
800549f0:  beq v0,zero,0x800549e4
800549f4:  _nop
800549f8:  jal 0x800566b4
800549fc:  _li a0,0x190
80054a00:  lui a0,0x8006
80054a04:  lw a0,0x52c0(a0)
80054a08:  nop
80054a0c:  lbu v0,0x0(a0)
80054a10:  lbu v1,0x44(s0)
80054a14:  nop
80054a18:  bne v1,zero,0x80054a30
80054a1c:  _andi s2,v0,0xff
80054a20:  sra v1,s2,0x4
80054a24:  li v0,0x8
80054a28:  beq v1,v0,0x80054a3c
80054a2c:  _li v0,0x22
80054a30:  sh s1,0xe(a0)
80054a34:  j 0x80054a40
80054a38:  _nop
80054a3c:  sh v0,0xe(a0)
80054a40:  lui v1,0x8006
80054a44:  lw v1,0x52bc(v1)
80054a48:  nop
80054a4c:  lw v0,0x0(v1)
80054a50:  nop
80054a54:  andi v0,v0,0x80
80054a58:  bne v0,zero,0x80054b10
80054a5c:  _nop
80054a60:  lui t1,0x1f80
80054a64:  ori t1,t1,0x1120
80054a68:  lui a2,0x1f80
80054a6c:  ori a2,a2,0x1128
80054a70:  lui t2,0x1
80054a74:  lui t0,0x1f80
80054a78:  ori t0,t0,0x1124
80054a7c:  move a3,v1
80054a80:  lui a0,0x800a
80054a84:  lw a0,0x4f08(a0)
80054a88:  lui a1,0x800a
80054a8c:  lw a1,0x4f0c(a1)
80054a90:  lhu v0,0x0(t1)
80054a94:  nop
80054a98:  andi v1,v0,0xffff
80054a9c:  sltu v0,v1,a0
80054aa0:  beq v0,zero,0x80054ac8
80054aa4:  _nop
80054aa8:  lhu v0,0x0(a2)
80054aac:  nop
80054ab0:  beq v0,zero,0x80054ac4
80054ab4:  _nop
80054ab8:  lhu v0,0x0(a2)
80054abc:  j 0x80054ac8
80054ac0:  _addu v1,v0,v1
80054ac4:  addu v1,v1,t2
80054ac8:  lhu v0,0x0(t0)
80054acc:  nop
80054ad0:  andi v0,v0,0x200
80054ad4:  beq v0,zero,0x80054ae8
80054ad8:  _subu v0,v1,a0
80054adc:  sltu v0,v0,a1
80054ae0:  beq v0,zero,0x80054b9c
80054ae4:  _li v0,-0x2
80054ae8:  subu v0,v1,a0
80054aec:  srl v0,v0,0x3
80054af0:  sltu v0,v0,a1
80054af4:  beq v0,zero,0x80054b9c
80054af8:  _li v0,-0x2
80054afc:  lw v0,0x0(a3)
80054b00:  nop
80054b04:  andi v0,v0,0x80
80054b08:  beq v0,zero,0x80054a90
80054b0c:  _nop
80054b10:  lbu v1,0xe8(s0)
80054b14:  li v0,0x8
80054b18:  beq v1,v0,0x80054b4c
80054b1c:  _li v0,0x2
80054b20:  lui v1,0x8006
80054b24:  lw v1,0x52a0(v1)
80054b28:  nop
80054b2c:  bne v1,v0,0x80054b4c
80054b30:  _nop
80054b34:  jal 0x800566b4
80054b38:  _li a0,0x3c
80054b3c:  jal 0x800566d4
80054b40:  _nop
80054b44:  beq v0,zero,0x80054b3c
80054b48:  _nop
80054b4c:  lui v0,0x8006
80054b50:  lw v0,0x52c0(v0)
80054b54:  nop
80054b58:  sb s3,0x0(v0)
80054b5c:  lbu v0,0x45(s0)
80054b60:  lbu v1,0x44(s0)
80054b64:  addiu v0,v0,0x1
80054b68:  sb v0,0x45(s0)
80054b6c:  li v0,0xff
80054b70:  beq v1,v0,0x80054b8c
80054b74:  _nop
80054b78:  lbu v1,0x44(s0)
80054b7c:  lw v0,0x3c(s0)
80054b80:  nop
80054b84:  addu v0,v0,v1
80054b88:  sb s2,0x0(v0)
80054b8c:  lbu v1,0x44(s0)
80054b90:  move v0,s2
80054b94:  addiu v1,v1,0x1
80054b98:  sb v1,0x44(s0)
80054b9c:  lw ra,0x20(sp)
80054ba0:  lw s3,0x1c(sp)
80054ba4:  lw s2,0x18(sp)
80054ba8:  lw s1,0x14(sp)
80054bac:  lw s0,0x10(sp)
80054bb0:  jr ra
80054bb4:  _addiu sp,sp,0x28
