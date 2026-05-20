# addr: 0x80101a98  name: FUN_80101a98
80101a98:  addiu sp,sp,-0x20
80101a9c:  sw s0,0x18(sp)
80101aa0:  move s0,a0
80101aa4:  li v0,0x3
80101aa8:  beq a1,v0,0x80101ac4
80101aac:  _sw ra,0x1c(sp)
80101ab0:  li v0,0x8
80101ab4:  beq a1,v0,0x80101ae0
80101ab8:  _clear v0
80101abc:  slti a0,zero,0x6d3
80101ac0:  nop
80101ac4:  move a0,s0
80101ac8:  jal 0x8002239c
80101acc:  _move a1,a2
80101ad0:  bne v0,zero,0x80101af4
80101ad4:  _clear v0
80101ad8:  slti a0,zero,0x6d3
80101adc:  nop
80101ae0:  move a0,s0
80101ae4:  jal 0x80022320
80101ae8:  _move a1,a2
80101aec:  beq v0,zero,0x80101b4c
80101af0:  _clear v0
80101af4:  lw v1,0x48(s0)
80101af8:  nop
80101afc:  bgez v1,0x80101b10
80101b00:  _sra v0,v1,0x10
80101b04:  ori v0,zero,0xffff
80101b08:  addu v1,v1,v0
80101b0c:  sra v0,v1,0x10
80101b10:  sh v0,0x10(sp)
80101b14:  lw v1,0x50(s0)
80101b18:  nop
80101b1c:  bgez v1,0x80101b2c
80101b20:  _addiu a0,sp,0x10
80101b24:  ori v0,zero,0xffff
80101b28:  addu v1,v1,v0
80101b2c:  ori a1,zero,0x8f80
80101b30:  sra v0,v1,0x10
80101b34:  sh v0,0x12(sp)
80101b38:  li v0,0x1
80101b3c:  sh v0,0x14(sp)
80101b40:  jal 0x80024718
80101b44:  _sh v0,0x16(sp)
80101b48:  clear v0
80101b4c:  lw ra,0x1c(sp)
80101b50:  lw s0,0x18(sp)
80101b54:  jr ra
80101b58:  _addiu sp,sp,0x20
