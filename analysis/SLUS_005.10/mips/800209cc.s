# addr: 0x800209cc  name: FUN_800209cc
800209cc:  addiu sp,sp,-0x38
800209d0:  sw s0,0x18(sp)
800209d4:  move s0,a0
800209d8:  sw ra,0x30(sp)
800209dc:  sw s5,0x2c(sp)
800209e0:  sw s4,0x28(sp)
800209e4:  sw s3,0x24(sp)
800209e8:  sw s2,0x20(sp)
800209ec:  sw s1,0x1c(sp)
800209f0:  lw v1,0x0(s0)
800209f4:  move s3,a1
800209f8:  move s4,a2
800209fc:  li v0,0x1
80020a00:  lw s2,0x48(sp)
80020a04:  beq v1,v0,0x80020a34
80020a08:  _move s5,a3
80020a0c:  beq v1,zero,0x80020a24
80020a10:  _li v0,0x2
80020a14:  beq v1,v0,0x80020a64
80020a18:  _nop
80020a1c:  j 0x80020aac
80020a20:  _nop
80020a24:  jal 0x800206f0
80020a28:  _addiu a0,s0,0x4
80020a2c:  j 0x80020aac
80020a30:  _nop
80020a34:  lw s1,0x4(s0)
80020a38:  nop
80020a3c:  slt v0,s3,s1
80020a40:  beq v0,zero,0x80020a5c
80020a44:  _move a1,s3
80020a48:  sw s2,0x10(sp)
80020a4c:  lw a0,0x8(s0)
80020a50:  move a2,s4
80020a54:  jal 0x800209cc
80020a58:  _move a3,s5
80020a5c:  j 0x80020a90
80020a60:  _slt v0,s1,s4
80020a64:  lw s1,0x4(s0)
80020a68:  nop
80020a6c:  slt v0,s5,s1
80020a70:  beq v0,zero,0x80020a8c
80020a74:  _move a1,s3
80020a78:  sw s2,0x10(sp)
80020a7c:  lw a0,0x8(s0)
80020a80:  move a2,s4
80020a84:  jal 0x800209cc
80020a88:  _move a3,s5
80020a8c:  slt v0,s1,s2
80020a90:  beq v0,zero,0x80020aac
80020a94:  _move a1,s3
80020a98:  sw s2,0x10(sp)
80020a9c:  lw a0,0xc(s0)
80020aa0:  move a2,s4
80020aa4:  jal 0x800209cc
80020aa8:  _move a3,s5
80020aac:  lw ra,0x30(sp)
80020ab0:  lw s5,0x2c(sp)
80020ab4:  lw s4,0x28(sp)
80020ab8:  lw s3,0x24(sp)
80020abc:  lw s2,0x20(sp)
80020ac0:  lw s1,0x1c(sp)
80020ac4:  lw s0,0x18(sp)
80020ac8:  jr ra
80020acc:  _addiu sp,sp,0x38
