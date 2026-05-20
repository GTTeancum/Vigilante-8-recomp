# addr: 0x80021a30  name: FUN_80021a30
80021a30:  addiu sp,sp,-0x28
80021a34:  sw s0,0x10(sp)
80021a38:  move s0,a0
80021a3c:  sw ra,0x24(sp)
80021a40:  sw s4,0x20(sp)
80021a44:  sw s3,0x1c(sp)
80021a48:  sw s2,0x18(sp)
80021a4c:  sw s1,0x14(sp)
80021a50:  lw v1,0x0(s0)
80021a54:  move s3,a1
80021a58:  move s4,a2
80021a5c:  li v0,0x1
80021a60:  beq v1,v0,0x80021a9c
80021a64:  _move s1,a3
80021a68:  beq v1,zero,0x80021a80
80021a6c:  _li v0,0x2
80021a70:  beq v1,v0,0x80021b00
80021a74:  _nop
80021a78:  j 0x80021b60
80021a7c:  _nop
80021a80:  addiu a0,s0,0x4
80021a84:  move a1,s3
80021a88:  move a2,s4
80021a8c:  jal 0x80021978
80021a90:  _move a3,s1
80021a94:  j 0x80021b60
80021a98:  _nop
80021a9c:  lw s2,0x4(s0)
80021aa0:  lw v0,0x0(s1)
80021aa4:  nop
80021aa8:  slt v0,v0,s2
80021aac:  beq v0,zero,0x80021acc
80021ab0:  _move a1,s3
80021ab4:  lw a0,0x8(s0)
80021ab8:  move a2,s4
80021abc:  jal 0x80021a30
80021ac0:  _move a3,s1
80021ac4:  bne v0,zero,0x80021b60
80021ac8:  _nop
80021acc:  lw v0,0x4(s1)
80021ad0:  nop
80021ad4:  slt v0,s2,v0
80021ad8:  beq v0,zero,0x80021af8
80021adc:  _move a1,s3
80021ae0:  lw a0,0xc(s0)
80021ae4:  move a2,s4
80021ae8:  jal 0x80021a30
80021aec:  _move a3,s1
80021af0:  bne v0,zero,0x80021b60
80021af4:  _nop
80021af8:  j 0x80021b60
80021afc:  _clear v0
80021b00:  lw s2,0x4(s0)
80021b04:  lw v0,0x8(s1)
80021b08:  nop
80021b0c:  slt v0,v0,s2
80021b10:  beq v0,zero,0x80021b30
80021b14:  _move a1,s3
80021b18:  lw a0,0x8(s0)
80021b1c:  move a2,s4
80021b20:  jal 0x80021a30
80021b24:  _move a3,s1
80021b28:  bne v0,zero,0x80021b60
80021b2c:  _nop
80021b30:  lw v0,0xc(s1)
80021b34:  nop
80021b38:  slt v0,s2,v0
80021b3c:  beq v0,zero,0x80021b5c
80021b40:  _move a1,s3
80021b44:  lw a0,0xc(s0)
80021b48:  move a2,s4
80021b4c:  jal 0x80021a30
80021b50:  _move a3,s1
80021b54:  bne v0,zero,0x80021b60
80021b58:  _nop
80021b5c:  clear v0
80021b60:  lw ra,0x24(sp)
80021b64:  lw s4,0x20(sp)
80021b68:  lw s3,0x1c(sp)
80021b6c:  lw s2,0x18(sp)
80021b70:  lw s1,0x14(sp)
80021b74:  lw s0,0x10(sp)
80021b78:  jr ra
80021b7c:  _addiu sp,sp,0x28
