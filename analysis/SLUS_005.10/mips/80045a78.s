# addr: 0x80045a78  name: _spu_t
80045a78:  sw a0,0x0(sp)
80045a7c:  sw a1,0x4(sp)
80045a80:  sw a2,0x8(sp)
80045a84:  sw a3,0xc(sp)
80045a88:  addiu sp,sp,-0x18
80045a8c:  sw s0,0x10(sp)
80045a90:  addiu s0,sp,0x1c
80045a94:  li a2,0x1
80045a98:  sw ra,0x14(sp)
80045a9c:  beq a0,a2,0x80045b04
80045aa0:  _sw a0,0x18(sp)
80045aa4:  slti v0,a0,0x2
80045aa8:  beq v0,zero,0x80045ac0
80045aac:  _li v0,0x2
80045ab0:  beq a0,zero,0x80045b70
80045ab4:  _clear v0
80045ab8:  j 0x80045ce8
80045abc:  _nop
80045ac0:  beq a0,v0,0x80045ad8
80045ac4:  _li v0,0x3
80045ac8:  beq a0,v0,0x80045bd8
80045acc:  _clear v0
80045ad0:  j 0x80045ce8
80045ad4:  _nop
80045ad8:  lw a0,0x1c(sp)
80045adc:  lui v0,0x8006
80045ae0:  lw v0,-0x1204(v0)
80045ae4:  lui v1,0x8006
80045ae8:  lw v1,-0x122c(v1)
80045aec:  srlv v0,a0,v0
80045af0:  lui at,0x8006
80045af4:  sh v0,-0x1214(at)
80045af8:  sh v0,0x1a6(v1)
80045afc:  j 0x80045ce8
80045b00:  _clear v0
80045b04:  lui a1,0x8006
80045b08:  lw a1,-0x122c(a1)
80045b0c:  lui a0,0x8006
80045b10:  lhu a0,-0x1214(a0)
80045b14:  lhu v0,0x1a6(a1)
80045b18:  lui at,0x8006
80045b1c:  sw zero,-0x11dc(at)
80045b20:  beq v0,a0,0x80045b48
80045b24:  _clear v1
80045b28:  addiu v1,v1,0x1
80045b2c:  sltiu v0,v1,0xf01
80045b30:  beq v0,zero,0x80045ce8
80045b34:  _li v0,-0x2
80045b38:  lhu v0,0x1a6(a1)
80045b3c:  nop
80045b40:  bne v0,a0,0x80045b2c
80045b44:  _addiu v1,v1,0x1
80045b48:  lui v1,0x8006
80045b4c:  lw v1,-0x122c(v1)
80045b50:  nop
80045b54:  lhu v0,0x1aa(v1)
80045b58:  nop
80045b5c:  andi v0,v0,0xffcf
80045b60:  ori v0,v0,0x20
80045b64:  sh v0,0x1aa(v1)
80045b68:  j 0x80045ce8
80045b6c:  _clear v0
80045b70:  lui a1,0x8006
80045b74:  lw a1,-0x122c(a1)
80045b78:  lui a0,0x8006
80045b7c:  lhu a0,-0x1214(a0)
80045b80:  lhu v0,0x1a6(a1)
80045b84:  lui at,0x8006
80045b88:  sw a2,-0x11dc(at)
80045b8c:  beq v0,a0,0x80045bb4
80045b90:  _clear v1
80045b94:  addiu v1,v1,0x1
80045b98:  sltiu v0,v1,0xf01
80045b9c:  beq v0,zero,0x80045ce8
80045ba0:  _li v0,-0x2
80045ba4:  lhu v0,0x1a6(a1)
80045ba8:  nop
80045bac:  bne v0,a0,0x80045b98
80045bb0:  _addiu v1,v1,0x1
80045bb4:  lui v1,0x8006
80045bb8:  lw v1,-0x122c(v1)
80045bbc:  nop
80045bc0:  lhu v0,0x1aa(v1)
80045bc4:  nop
80045bc8:  ori v0,v0,0x30
80045bcc:  sh v0,0x1aa(v1)
80045bd0:  j 0x80045ce8
80045bd4:  _clear v0
80045bd8:  lui v0,0x8006
80045bdc:  lw v0,-0x11dc(v0)
80045be0:  nop
80045be4:  bne v0,a2,0x80045bf0
80045be8:  _li a0,0x20
80045bec:  li a0,0x30
80045bf0:  lui a1,0x8006
80045bf4:  lw a1,-0x122c(a1)
80045bf8:  clear v1
80045bfc:  lhu v0,0x1aa(a1)
80045c00:  andi a0,a0,0xffff
80045c04:  andi v0,v0,0x30
80045c08:  beq v0,a0,0x80045c30
80045c0c:  _addiu v1,v1,0x1
80045c10:  sltiu v0,v1,0xf01
80045c14:  beq v0,zero,0x80045ce8
80045c18:  _li v0,-0x2
80045c1c:  lhu v0,0x1aa(a1)
80045c20:  nop
80045c24:  andi v0,v0,0x30
80045c28:  bne v0,a0,0x80045c10
80045c2c:  _addiu v1,v1,0x1
80045c30:  lui v1,0x8006
80045c34:  lw v1,-0x11dc(v1)
80045c38:  li v0,0x1
80045c3c:  bne v1,v0,0x80045c54
80045c40:  _nop
80045c44:  jal 0x80045f84
80045c48:  _addiu s0,s0,0x4
80045c4c:  j 0x80045c60
80045c50:  _lui a2,0x100
80045c54:  jal 0x80045f5c
80045c58:  _addiu s0,s0,0x4
80045c5c:  lui a2,0x100
