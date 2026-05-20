# addr: 0x80020ad0  name: FUN_80020ad0
80020ad0:  addiu sp,sp,-0x20
80020ad4:  sw ra,0x18(sp)
80020ad8:  sw s1,0x14(sp)
80020adc:  sw s0,0x10(sp)
80020ae0:  lw a0,0x0(a0)
80020ae4:  nop
80020ae8:  lw a1,0x0(a0)
80020aec:  nop
80020af0:  beq a1,zero,0x80020bd4
80020af4:  _li s1,0x1
80020af8:  lw s0,0x8(a0)
80020afc:  nop
80020b00:  lw v1,0x30(s0)
80020b04:  nop
80020b08:  beq v1,zero,0x80020bc4
80020b0c:  _move a0,a1
80020b10:  lw v0,0x1c(v1)
80020b14:  nop
80020b18:  beq v0,zero,0x80020bc4
80020b1c:  _nop
80020b20:  lw v0,0x20(v1)
80020b24:  nop
80020b28:  beq v0,zero,0x80020bc4
80020b2c:  _nop
80020b30:  lw v0,0x4(gp)
80020b34:  nop
80020b38:  subu v0,s1,v0
80020b3c:  sll v0,v0,0x2
80020b40:  addu v0,v1,v0
80020b44:  lw a0,0x1c(v0)
80020b48:  jal 0x80045088
80020b4c:  _nop
80020b50:  lw v0,0x4(gp)
80020b54:  lw v1,0x30(s0)
80020b58:  subu v0,s1,v0
80020b5c:  sll a0,v0,0x2
80020b60:  addu v1,v1,a0
80020b64:  sw zero,0x1c(v1)
80020b68:  lw v1,0x68(s0)
80020b6c:  nop
80020b70:  beq v1,zero,0x80020bd8
80020b74:  _li v0,0x1
80020b78:  lw v0,0x1c(v1)
80020b7c:  nop
80020b80:  beq v0,zero,0x80020bd8
80020b84:  _li v0,0x1
80020b88:  lw v0,0x20(v1)
80020b8c:  nop
80020b90:  beq v0,zero,0x80020bbc
80020b94:  _addu v0,v1,a0
80020b98:  lw a0,0x1c(v0)
80020b9c:  jal 0x80045088
80020ba0:  _nop
80020ba4:  lw v0,0x4(gp)
80020ba8:  lw v1,0x68(s0)
80020bac:  subu v0,s1,v0
80020bb0:  sll v0,v0,0x2
80020bb4:  addu v1,v1,v0
80020bb8:  sw zero,0x1c(v1)
80020bbc:  j 0x80020bd8
80020bc0:  _li v0,0x1
80020bc4:  lw a1,0x0(a1)
80020bc8:  nop
80020bcc:  bne a1,zero,0x80020af8
80020bd0:  _nop
80020bd4:  clear v0
80020bd8:  lw ra,0x18(sp)
80020bdc:  lw s1,0x14(sp)
80020be0:  lw s0,0x10(sp)
80020be4:  jr ra
80020be8:  _addiu sp,sp,0x20
