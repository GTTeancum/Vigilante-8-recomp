# addr: 0x80100a30  name: FUN_80100a30
80100a30:  addiu sp,sp,-0x50
80100a34:  sw s0,0x40(sp)
80100a38:  move s0,a0
80100a3c:  sw s1,0x44(sp)
80100a40:  move s1,a2
80100a44:  beq a1,zero,0x80100a60
80100a48:  _sw ra,0x48(sp)
80100a4c:  li v0,0x3
80100a50:  beq a1,v0,0x80100cd8
80100a54:  _clear v0
80100a58:  slti a0,zero,0x399
80100a5c:  nop
80100a60:  lw v0,0x24(s0)
80100a64:  nop
80100a68:  sw v0,0x10(sp)
80100a6c:  lw v0,0x28(s0)
80100a70:  lw v1,0x54(s0)
80100a74:  addiu a1,sp,0x10
80100a78:  addu v0,v0,v1
80100a7c:  sw v0,0x4(a1)
80100a80:  lw v0,0x2c(s0)
80100a84:  move a0,s0
80100a88:  addiu s1,sp,0x20
80100a8c:  move a2,s1
80100a90:  clear a3
80100a94:  jal 0x8001d748
80100a98:  _sw v0,0x8(a1)
80100a9c:  lw v1,0x14(sp)
80100aa0:  move t1,v0
80100aa4:  addiu v1,v1,0x800
80100aa8:  slt v1,t1,v1
80100aac:  beq v1,zero,0x80100c58
80100ab0:  _addiu a0,s0,0x80
80100ab4:  lh a1,0x20(sp)
80100ab8:  lw a3,0x80(s0)
80100abc:  nop
80100ac0:  mult a3,a1
80100ac4:  lhu v0,0x2(s1)
80100ac8:  lw v1,0x4(a0)
80100acc:  mflo t0
80100ad0:  sll v0,v0,0x10
80100ad4:  sra v0,v0,0x10
80100ad8:  mult v1,v0
80100adc:  lhu v0,0x4(s1)
80100ae0:  lw v1,0x8(a0)
80100ae4:  mflo a2
80100ae8:  sll v0,v0,0x10
80100aec:  sra v0,v0,0x10
80100af0:  mult v1,v0
80100af4:  addu v0,t0,a2
80100af8:  mflo v1
80100afc:  addu v0,v0,v1
80100b00:  bgez v0,0x80100b10
80100b04:  _sra a0,v0,0xb
80100b08:  addiu v0,v0,0x7ff
80100b0c:  sra a0,v0,0xb
80100b10:  bgez a0,0x80100bac
80100b14:  _sll v1,a1,0x1
80100b18:  mult a0,a1
80100b1c:  mflo v0
80100b20:  bgez v0,0x80100b2c
80100b24:  _nop
80100b28:  addiu v0,v0,0xfff
80100b2c:  sra v0,v0,0xc
80100b30:  subu v0,a3,v0
80100b34:  sw v0,0x80(s0)
80100b38:  lh v0,0x22(sp)
80100b3c:  nop
80100b40:  mult a0,v0
80100b44:  mflo v1
80100b48:  bgez v1,0x80100b54
80100b4c:  _nop
80100b50:  addiu v1,v1,0xfff
80100b54:  lw v0,0x84(s0)
80100b58:  sra v1,v1,0xc
80100b5c:  subu v0,v0,v1
80100b60:  srl v1,v0,0x1f
80100b64:  addu v0,v0,v1
80100b68:  sra v0,v0,0x1
80100b6c:  sw v0,0x84(s0)
80100b70:  lh v0,0x24(sp)
80100b74:  nop
80100b78:  mult a0,v0
80100b7c:  mflo v1
80100b80:  bgez v1,0x80100b8c
80100b84:  _nop
80100b88:  addiu v1,v1,0xfff
80100b8c:  lw v0,0x88(s0)
80100b90:  lw a0,0x54(s0)
80100b94:  sra v1,v1,0xc
80100b98:  subu v0,v0,v1
80100b9c:  subu a0,t1,a0
80100ba0:  sw v0,0x88(s0)
80100ba4:  slti a0,zero,0x303
80100ba8:  sw a0,0x28(s0)
80100bac:  addu v1,v1,a1
80100bb0:  sll v0,v1,0x4
80100bb4:  subu v0,v0,v1
80100bb8:  sll v0,v0,0x1
80100bbc:  bgez v0,0x80100bc8
80100bc0:  _nop
80100bc4:  addiu v0,v0,0xfff
80100bc8:  sra v0,v0,0xc
80100bcc:  addu v0,a3,v0
80100bd0:  sw v0,0x80(s0)
80100bd4:  lh v0,0x24(sp)
80100bd8:  nop
80100bdc:  sll v1,v0,0x1
80100be0:  addu v1,v1,v0
80100be4:  sll v0,v1,0x4
80100be8:  subu v0,v0,v1
80100bec:  sll v1,v0,0x1
80100bf0:  bgez v1,0x80100bfc
80100bf4:  _nop
80100bf8:  addiu v1,v1,0xfff
80100bfc:  lw v0,0x88(s0)
80100c00:  sra v1,v1,0xc
80100c04:  addu v0,v0,v1
80100c08:  sw v0,0x88(s0)
80100c0c:  lw v0,0x88(s0)
80100c10:  lhu v1,0x94(s0)
80100c14:  subu v0,zero,v0
80100c18:  mult v0,v1
80100c1c:  mflo a0
80100c20:  bgez a0,0x80100c2c
80100c24:  _nop
80100c28:  addiu a0,a0,0xfff
80100c2c:  lw v0,0x80(s0)
80100c30:  nop
80100c34:  mult v0,v1
80100c38:  sra v0,a0,0xc
80100c3c:  mflo v1
80100c40:  bgez v1,0x80100c4c
80100c44:  _sh v0,0x8c(s0)
80100c48:  addiu v1,v1,0xfff
80100c4c:  sra v0,v1,0xc
80100c50:  slti a0,zero,0x31a
80100c54:  sh v0,0x90(s0)
80100c58:  lw v0,0x84(s0)
80100c5c:  nop
80100c60:  addiu v0,v0,0x5a
80100c64:  sw v0,0x84(s0)
80100c68:  addiu s1,s0,0x10
80100c6c:  move a0,s1
80100c70:  move a1,s1
80100c74:  jal 0x800172b4
80100c78:  _addiu a2,s0,0x8c
80100c7c:  lw v1,0x24(s0)
80100c80:  lw a2,0x80(s0)
80100c84:  lw a0,0x28(s0)
80100c88:  lw a3,0x84(s0)
80100c8c:  lw a1,0x2c(s0)
80100c90:  lw t0,0x88(s0)
80100c94:  lbu t1,0x9(s0)
80100c98:  lui v0,0x8006
80100c9c:  lw v0,0x5310(v0)
80100ca0:  addu v1,v1,a2
80100ca4:  addu a0,a0,a3
80100ca8:  addu a1,a1,t0
80100cac:  subu v0,v0,t1
80100cb0:  andi v0,v0,0xf
80100cb4:  sw v1,0x24(s0)
80100cb8:  sw a0,0x28(s0)
80100cbc:  bne v0,zero,0x80100e60
80100cc0:  _sw a1,0x2c(s0)
80100cc4:  move a0,s1
80100cc8:  jal 0x8004c934
80100ccc:  _move a1,a0
80100cd0:  slti a0,zero,0x399
80100cd4:  clear v0
80100cd8:  lw a0,0x0(s1)
80100cdc:  nop
80100ce0:  lbu v1,0x4(a0)
80100ce4:  li v0,0x7
80100ce8:  bne v1,v0,0x80100d0c
80100cec:  _li v0,0x2
80100cf0:  lhu a1,0xc(a0)
80100cf4:  jal 0x80022320
80100cf8:  _move a0,s0
80100cfc:  beq v0,zero,0x80100e64
80100d00:  _clear v0
80100d04:  slti a0,zero,0x358
80100d08:  nop
80100d0c:  bne v1,v0,0x80100d70
80100d10:  _addiu a1,sp,0x30
80100d14:  lw v0,0x24(a0)
80100d18:  lw v1,0x24(s0)
80100d1c:  nop
80100d20:  subu v0,v0,v1
80100d24:  sll v0,v0,0x3
80100d28:  sw v0,0x30(sp)
80100d2c:  lw v0,0x28(a0)
80100d30:  lw v1,0x28(s0)
80100d34:  nop
80100d38:  subu v0,v0,v1
80100d3c:  sll v0,v0,0x3
80100d40:  sw v0,0x4(a1)
80100d44:  lw v0,0x2c(a0)
80100d48:  lw v1,0x2c(s0)
80100d4c:  move a0,s0
80100d50:  subu v0,v0,v1
80100d54:  sll v0,v0,0x3
80100d58:  jal 0x8003fc50
80100d5c:  _sw v0,0x8(a1)
80100d60:  jal 0x800205f8
80100d64:  _move a0,s0
80100d68:  slti a0,zero,0x399
80100d6c:  li v0,-0x1
80100d70:  move a0,s0
80100d74:  jal 0x8001f5a0
80100d78:  _move a1,s1
80100d7c:  lh t0,0x20(s1)
80100d80:  lw t1,0x80(s0)
80100d84:  nop
80100d88:  mult t1,t0
80100d8c:  addiu a1,s1,0x20
80100d90:  lh v1,0x2(a1)
80100d94:  addiu a0,s0,0x80
80100d98:  lw v0,0x4(a0)
80100d9c:  mflo a3
80100da0:  nop
80100da4:  nop
80100da8:  mult v0,v1
80100dac:  lh v1,0x4(a1)
80100db0:  lw v0,0x8(a0)
80100db4:  mflo a2
80100db8:  nop
80100dbc:  nop
80100dc0:  mult v0,v1
80100dc4:  addu v0,a3,a2
80100dc8:  mflo v1
80100dcc:  addu v0,v0,v1
80100dd0:  bgez v0,0x80100de0
80100dd4:  _sra a0,v0,0xb
80100dd8:  addiu v0,v0,0x7ff
80100ddc:  sra a0,v0,0xb
80100de0:  bgez a0,0x80100e64
80100de4:  _clear v0
80100de8:  mult a0,t0
80100dec:  mflo v0
80100df0:  bgez v0,0x80100dfc
80100df4:  _nop
80100df8:  addiu v0,v0,0xfff
80100dfc:  sra v0,v0,0xc
80100e00:  subu v0,t1,v0
80100e04:  sw v0,0x80(s0)
80100e08:  lh v0,0x22(s1)
80100e0c:  nop
80100e10:  mult a0,v0
80100e14:  mflo v1
80100e18:  bgez v1,0x80100e24
80100e1c:  _nop
80100e20:  addiu v1,v1,0xfff
80100e24:  lw v0,0x84(s0)
80100e28:  sra v1,v1,0xc
80100e2c:  subu v0,v0,v1
80100e30:  sw v0,0x84(s0)
80100e34:  lh v0,0x24(s1)
80100e38:  nop
80100e3c:  mult a0,v0
80100e40:  mflo v1
80100e44:  bgez v1,0x80100e50
80100e48:  _nop
80100e4c:  addiu v1,v1,0xfff
80100e50:  lw v0,0x88(s0)
80100e54:  sra v1,v1,0xc
80100e58:  subu v0,v0,v1
80100e5c:  sw v0,0x88(s0)
80100e60:  clear v0
80100e64:  lw ra,0x48(sp)
80100e68:  lw s1,0x44(sp)
80100e6c:  lw s0,0x40(sp)
80100e70:  jr ra
80100e74:  _addiu sp,sp,0x50
