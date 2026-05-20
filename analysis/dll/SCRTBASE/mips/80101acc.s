# addr: 0x80101acc  name: FUN_80101acc
80101acc:  addiu sp,sp,-0x28
80101ad0:  sw s2,0x18(sp)
80101ad4:  move s2,a0
80101ad8:  li v1,0x2
80101adc:  sw ra,0x20(sp)
80101ae0:  sw s3,0x1c(sp)
80101ae4:  sw s1,0x14(sp)
80101ae8:  beq a1,v1,0x80101c10
80101aec:  _sw s0,0x10(sp)
80101af0:  sltiu v0,a1,0x3
80101af4:  beq v0,zero,0x80101b0c
80101af8:  _li v0,0x1
80101afc:  beq a1,v0,0x80101c2c
80101b00:  _li v0,0x3
80101b04:  slti a0,zero,0x70e
80101b08:  nop
80101b0c:  li v0,0x3
80101b10:  beq a1,v0,0x80101b28
80101b14:  _li v0,0x8
80101b18:  beq a1,v0,0x80101bf0
80101b1c:  _move a0,s2
80101b20:  slti a0,zero,0x70e
80101b24:  nop
80101b28:  lw s1,0xc(a2)
80101b2c:  nop
80101b30:  lbu v0,0x4(s1)
80101b34:  nop
80101b38:  bne v0,a1,0x80101be0
80101b3c:  _move a0,s2
80101b40:  lw s3,0x0(a2)
80101b44:  nop
80101b48:  lbu v0,0x4(s3)
80101b4c:  nop
80101b50:  bne v0,v1,0x80101be0
80101b54:  _nop
80101b58:  lw v0,0x80(s3)
80101b5c:  nop
80101b60:  bgez v0,0x80101b6c
80101b64:  _li a0,0x74
80101b68:  li a0,0x75
80101b6c:  jal 0x80021808
80101b70:  _nop
80101b74:  move s0,v0
80101b78:  lw v0,0x60(s0)
80101b7c:  nop
80101b80:  lhu a1,0x0(v0)
80101b84:  jal 0x8001f9cc
80101b88:  _move a0,s0
80101b8c:  move a0,s0
80101b90:  li a1,0x258
80101b94:  li v0,0x1
80101b98:  jal 0x80020890
80101b9c:  _sb v0,0x8(a0)
80101ba0:  move a0,s2
80101ba4:  jal 0x80020890
80101ba8:  _li a1,0x258
80101bac:  lw v0,0x0(s1)
80101bb0:  nop
80101bb4:  ori v0,v0,0x20
80101bb8:  jal 0x8004410c
80101bbc:  _sw v0,0x0(s1)
80101bc0:  lui v1,0x8006
80101bc4:  lw a1,0x58fc(v1)
80101bc8:  move a0,v0
80101bcc:  li a2,0x14
80101bd0:  jal 0x800447e8
80101bd4:  _addiu a3,s3,0x24
80101bd8:  slti a0,zero,0x70e
80101bdc:  nop
80101be0:  jal 0x8002239c
80101be4:  _move a1,a2
80101be8:  slti a0,zero,0x6fe
80101bec:  nop
80101bf0:  jal 0x80022320
80101bf4:  _move a1,a2
80101bf8:  beq v0,zero,0x80101c38
80101bfc:  _nop
80101c00:  jal 0x80020844
80101c04:  _move a0,s2
80101c08:  slti a0,zero,0x70e
80101c0c:  nop
80101c10:  lw v0,0x38(s2)
80101c14:  nop
80101c18:  lw v1,0x0(v0)
80101c1c:  li a0,-0x21
80101c20:  and v1,v1,a0
80101c24:  slti a0,zero,0x70e
80101c28:  sw v1,0x0(v0)
80101c2c:  lw v1,0x38(s2)
80101c30:  nop
80101c34:  sb v0,0x4(v1)
80101c38:  lw ra,0x20(sp)
80101c3c:  lw s3,0x1c(sp)
80101c40:  lw s2,0x18(sp)
80101c44:  lw s1,0x14(sp)
80101c48:  lw s0,0x10(sp)
80101c4c:  clear v0
80101c50:  jr ra
80101c54:  _addiu sp,sp,0x28
