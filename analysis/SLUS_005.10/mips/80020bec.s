# addr: 0x80020bec  name: FUN_80020bec
80020bec:  addiu sp,sp,-0x20
80020bf0:  sw s1,0x14(sp)
80020bf4:  move s1,a0
80020bf8:  sw ra,0x1c(sp)
80020bfc:  sw s2,0x18(sp)
80020c00:  sw s0,0x10(sp)
80020c04:  lw v1,0x0(s1)
80020c08:  li v0,0x1
80020c0c:  beq v1,v0,0x80020c40
80020c10:  _move s2,a1
80020c14:  beq v1,zero,0x80020c2c
80020c18:  _li v0,0x2
80020c1c:  beq v1,v0,0x80020cb4
80020c20:  _nop
80020c24:  j 0x80020d24
80020c28:  _nop
80020c2c:  addiu a0,s1,0x4
80020c30:  jal 0x80020ad0
80020c34:  _move a1,s2
80020c38:  j 0x80020d24
80020c3c:  _nop
80020c40:  lw a0,0x4(s1)
80020c44:  lw v1,0x4(s2)
80020c48:  lw v0,0x0(s2)
80020c4c:  subu v1,a0,v1
80020c50:  subu v0,v0,a0
80020c54:  slt v0,v0,v1
80020c58:  beq v0,zero,0x80020c80
80020c5c:  _clear s0
80020c60:  lw a0,0x8(s1)
80020c64:  jal 0x80020bec
80020c68:  _move a1,s2
80020c6c:  bne v0,zero,0x80020ca8
80020c70:  _nop
80020c74:  lw a0,0xc(s1)
80020c78:  j 0x80020c98
80020c7c:  _nop
80020c80:  lw a0,0xc(s1)
80020c84:  jal 0x80020bec
80020c88:  _move a1,s2
80020c8c:  bne v0,zero,0x80020ca8
80020c90:  _nop
80020c94:  lw a0,0x8(s1)
80020c98:  jal 0x80020bec
80020c9c:  _move a1,s2
80020ca0:  beq v0,zero,0x80020d24
80020ca4:  _move v0,s0
80020ca8:  li s0,0x1
80020cac:  j 0x80020d24
80020cb0:  _move v0,s0
80020cb4:  lw a0,0x4(s1)
80020cb8:  lw v1,0xc(s2)
80020cbc:  lw v0,0x8(s2)
80020cc0:  subu v1,a0,v1
80020cc4:  subu v0,v0,a0
80020cc8:  slt v0,v0,v1
80020ccc:  beq v0,zero,0x80020cf4
80020cd0:  _clear s0
80020cd4:  lw a0,0x8(s1)
80020cd8:  jal 0x80020bec
80020cdc:  _move a1,s2
80020ce0:  bne v0,zero,0x80020d1c
80020ce4:  _nop
80020ce8:  lw a0,0xc(s1)
80020cec:  j 0x80020d0c
80020cf0:  _nop
80020cf4:  lw a0,0xc(s1)
80020cf8:  jal 0x80020bec
80020cfc:  _move a1,s2
80020d00:  bne v0,zero,0x80020d1c
80020d04:  _nop
80020d08:  lw a0,0x8(s1)
80020d0c:  jal 0x80020bec
80020d10:  _move a1,s2
80020d14:  beq v0,zero,0x80020d24
80020d18:  _move v0,s0
80020d1c:  li s0,0x1
80020d20:  move v0,s0
80020d24:  lw ra,0x1c(sp)
80020d28:  lw s2,0x18(sp)
80020d2c:  lw s1,0x14(sp)
80020d30:  lw s0,0x10(sp)
80020d34:  jr ra
80020d38:  _addiu sp,sp,0x20
