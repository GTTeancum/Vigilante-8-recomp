# addr: 0x80054bb8  name: FUN_80054bb8
80054bb8:  addiu sp,sp,-0x18
80054bbc:  lui v1,0x8006
80054bc0:  lw v1,0x52bc(v1)
80054bc4:  lui a0,0x8006
80054bc8:  lw a0,0x52c0(a0)
80054bcc:  li v0,-0x81
80054bd0:  sw ra,0x10(sp)
80054bd4:  sw v0,0x0(v1)
80054bd8:  lhu v0,0x4(a0)
80054bdc:  nop
80054be0:  andi v0,v0,0x80
80054be4:  beq v0,zero,0x80054c1c
80054be8:  _nop
80054bec:  jal 0x800566d4
80054bf0:  _nop
80054bf4:  bne v0,zero,0x80054c38
80054bf8:  _clear v0
80054bfc:  lui v0,0x8006
80054c00:  lw v0,0x52c0(v0)
80054c04:  nop
80054c08:  lhu v0,0x4(v0)
80054c0c:  nop
80054c10:  andi v0,v0,0x80
80054c14:  bne v0,zero,0x80054bec
80054c18:  _nop
80054c1c:  lui a0,0x8006
80054c20:  lw a0,0x52c0(a0)
80054c24:  nop
80054c28:  lhu v1,0xa(a0)
80054c2c:  li v0,0x1
80054c30:  ori v1,v1,0x10
80054c34:  sh v1,0xa(a0)
80054c38:  lw ra,0x10(sp)
80054c3c:  addiu sp,sp,0x18
80054c40:  jr ra
80054c44:  _nop
