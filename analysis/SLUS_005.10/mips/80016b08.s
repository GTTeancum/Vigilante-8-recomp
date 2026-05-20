# addr: 0x80016b08  name: FUN_80016b08
80016b08:  addiu sp,sp,-0x20
80016b0c:  sw s1,0x14(sp)
80016b10:  move s1,a0
80016b14:  sw s0,0x10(sp)
80016b18:  sw ra,0x18(sp)
80016b1c:  jal 0x80016a20
80016b20:  _move s0,a1
80016b24:  move a0,v0
80016b28:  beq a0,zero,0x80016bb4
80016b2c:  _nop
80016b30:  gte_ldLZCS a0
80016b34:  nop
80016b38:  nop
80016b3c:  gte_stLZCR v0
80016b40:  li a1,0xc
80016b44:  addiu v1,v0,-0x1
80016b48:  slt v0,v1,a1
80016b4c:  beq v0,zero,0x80016b58
80016b50:  _nop
80016b54:  move a1,v1
80016b58:  lw v1,0x0(s1)
80016b5c:  li v0,0xc
80016b60:  subu v0,v0,a1
80016b64:  srav a0,a0,v0
80016b68:  sllv v1,v1,a1
80016b6c:  div v1,a0
80016b70:  mflo v1
80016b74:  nop
80016b78:  sh v1,0x0(s0)
80016b7c:  lw v0,0x4(s1)
80016b80:  nop
80016b84:  sllv v0,v0,a1
80016b88:  div v0,a0
80016b8c:  mflo v0
80016b90:  nop
80016b94:  sh v0,0x2(s0)
80016b98:  lw v0,0x8(s1)
80016b9c:  nop
80016ba0:  sllv v0,v0,a1
80016ba4:  div v0,a0
80016ba8:  mflo v0
80016bac:  j 0x80016bc0
80016bb0:  _sh v0,0x4(s0)
80016bb4:  sh zero,0x4(s0)
80016bb8:  sh zero,0x2(s0)
80016bbc:  sh zero,0x0(s0)
80016bc0:  lw ra,0x18(sp)
80016bc4:  lw s1,0x14(sp)
80016bc8:  move v0,s0
80016bcc:  lw s0,0x10(sp)
80016bd0:  jr ra
80016bd4:  _addiu sp,sp,0x20
