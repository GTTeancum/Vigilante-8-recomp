# addr: 0x80031864  name: FUN_80031864
80031864:  addiu sp,sp,-0x40
80031868:  sw s1,0x2c(sp)
8003186c:  move s1,a0
80031870:  sw s2,0x30(sp)
80031874:  move s2,a2
80031878:  sltiu v0,a1,0xf
8003187c:  sw ra,0x38(sp)
80031880:  sw s3,0x34(sp)
80031884:  beq v0,zero,0x80031adc
80031888:  _sw s0,0x28(sp)
8003188c:  lui v0,0x8001
80031890:  addiu v0,v0,0x5c0
80031894:  sll v1,a1,0x2
80031898:  addu v1,v1,v0
8003189c:  lw v0,0x0(v1)
800318a0:  nop
800318a4:  jr v0
800318a8:  _nop
800318ac:  move a0,s1
800318b0:  jal 0x8003c538
800318b4:  _move a1,s2
800318b8:  j 0x80031ae0
800318bc:  _clear v0
800318c0:  lbu v0,0x8(s1)
800318c4:  li v1,-0x1
800318c8:  addiu v0,v0,-0x1
800318cc:  sb v0,0x8(s1)
800318d0:  sll v0,v0,0x18
800318d4:  sra v0,v0,0x18
800318d8:  bne v0,v1,0x80031ae0
800318dc:  _clear v0
800318e0:  lui v0,0x8007
800318e4:  lw a0,0x37d8(v0)
800318e8:  li a1,0x3
800318ec:  li a2,0x80
800318f0:  jal 0x8001ac44
800318f4:  _li a3,0x8
800318f8:  move a0,s2
800318fc:  move a1,s1
80031900:  li a2,0x4
80031904:  li a3,0x98
80031908:  move s3,v0
8003190c:  jal 0x80031300
80031910:  _sw s3,0x10(sp)
80031914:  move s0,v0
80031918:  li v0,0x280
8003191c:  sw v0,0x0(s0)
80031920:  lhu v0,0x11c(s2)
80031924:  nop
80031928:  beq v0,zero,0x80031934
8003192c:  _li v1,0x7
80031930:  li v1,0xe
80031934:  lui v0,0x8003
80031938:  addiu v0,v0,0x1634
8003193c:  sh v1,0xc(s0)
80031940:  sw v0,0x64(s0)
80031944:  lw v1,0x80(s2)
80031948:  nop
8003194c:  bgez v1,0x80031958
80031950:  _addiu a1,s0,0x88
80031954:  addiu v1,v1,0x7f
80031958:  lh v0,0x14(s0)
8003195c:  sra v1,v1,0x7
80031960:  sll v0,v0,0x2
80031964:  addu v1,v1,v0
80031968:  sw v1,0x88(s0)
8003196c:  lw v1,0x84(s2)
80031970:  nop
80031974:  bgez v1,0x80031980
80031978:  _nop
8003197c:  addiu v1,v1,0x7f
80031980:  lh v0,0x1a(s0)
80031984:  sra v1,v1,0x7
80031988:  sll v0,v0,0x2
8003198c:  addu v1,v1,v0
80031990:  sw v1,0x4(a1)
80031994:  lw v0,0x88(s2)
80031998:  nop
8003199c:  bgez v0,0x800319a8
800319a0:  _nop
800319a4:  addiu v0,v0,0x7f
800319a8:  lh v1,0x20(s0)
800319ac:  move a0,s0
800319b0:  sra v0,v0,0x7
800319b4:  sll v1,v1,0x2
800319b8:  addu v0,v0,v1
800319bc:  sw v0,0x8(a1)
800319c0:  li v0,0x2d
800319c4:  jal 0x800202f4
800319c8:  _sh v0,0x94(s0)
800319cc:  lui v0,0x8004
800319d0:  addiu v0,v0,-0x17f4
800319d4:  sw v0,0x64(s3)
800319d8:  lw v0,0x0(s2)
800319dc:  nop
800319e0:  andi v0,v0,0x4
800319e4:  bne v0,zero,0x800319f4
800319e8:  _nop
800319ec:  jal 0x800207c4
800319f0:  _move a0,s3
800319f4:  lb a0,0x5(s1)
800319f8:  nop
800319fc:  bne a0,zero,0x80031a18
80031a00:  _nop
80031a04:  jal 0x8004410c
80031a08:  _nop
80031a0c:  sb v0,0x5(s1)
80031a10:  sll v0,v0,0x18
80031a14:  sra a0,v0,0x18
80031a18:  lw a1,0x5f8(gp)
80031a1c:  li a2,0x24
80031a20:  jal 0x800447e8
80031a24:  _addiu a3,s0,0x48
80031a28:  lhu v0,0xc(s1)
80031a2c:  lhu v1,0xc(s1)
80031a30:  srl v0,v0,0x8
80031a34:  addiu v1,v1,0x20
80031a38:  sb v0,0x8(s1)
80031a3c:  j 0x80031adc
80031a40:  _sh v1,0xc(s1)
80031a44:  j 0x80031ae0
80031a48:  _ori v0,zero,0x8010
80031a4c:  lw a1,0xe4(s2)
80031a50:  addiu a0,s2,0x10
80031a54:  addiu a2,sp,0x18
80031a58:  jal 0x800435c0
80031a5c:  _addiu a1,a1,0x24
80031a60:  lw a0,0x18(sp)
80031a64:  lw a1,0x20(sp)
80031a68:  jal 0x8004ecd4
80031a6c:  _clear s0
80031a70:  sll v0,v0,0x14
80031a74:  sra v0,v0,0x14
80031a78:  bgez v0,0x80031a84
80031a7c:  _nop
80031a80:  subu v0,zero,v0
80031a84:  slti v0,v0,0x71
80031a88:  beq v0,zero,0x80031aa0
80031a8c:  _lui v0,0x7
80031a90:  lw v1,0x20(sp)
80031a94:  ori v0,v0,0xcfff
80031a98:  slt v0,v0,v1
80031a9c:  xori s0,v0,0x1
80031aa0:  j 0x80031ae0
80031aa4:  _move v0,s0
80031aa8:  lhu v0,0xc(s1)
80031aac:  li v1,0x500
80031ab0:  addiu a0,v0,-0x40
80031ab4:  slt v0,v1,a0
80031ab8:  beq v0,zero,0x80031ac4
80031abc:  _nop
80031ac0:  move v1,a0
80031ac4:  sh v1,0xc(s1)
80031ac8:  sb zero,0x5(s1)
80031acc:  j 0x80031adc
80031ad0:  _sb zero,0x8(s1)
80031ad4:  li v0,0x500
80031ad8:  sh v0,0xc(s1)
80031adc:  clear v0
80031ae0:  lw ra,0x38(sp)
80031ae4:  lw s3,0x34(sp)
80031ae8:  lw s2,0x30(sp)
80031aec:  lw s1,0x2c(sp)
80031af0:  lw s0,0x28(sp)
80031af4:  jr ra
80031af8:  _addiu sp,sp,0x40
