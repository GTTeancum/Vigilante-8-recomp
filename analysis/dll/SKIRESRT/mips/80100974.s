# addr: 0x80100974  name: FUN_80100974
80100974:  addiu sp,sp,-0x98
80100978:  sw s4,0x80(sp)
8010097c:  move s4,a0
80100980:  move v1,a1
80100984:  move a1,a2
80100988:  sltiu v0,v1,0x12
8010098c:  sw ra,0x90(sp)
80100990:  sw s7,0x8c(sp)
80100994:  sw s6,0x88(sp)
80100998:  sw s5,0x84(sp)
8010099c:  sw s3,0x7c(sp)
801009a0:  sw s2,0x78(sp)
801009a4:  sw s1,0x74(sp)
801009a8:  beq v0,zero,0x80101020
801009ac:  _sw s0,0x70(sp)
801009b0:  lui v0,0x8010
801009b4:  addiu v0,v0,0x78
801009b8:  sll v1,v1,0x2
801009bc:  addu v1,v1,v0
801009c0:  lw v0,0x0(v1)
801009c4:  nop
801009c8:  jr v0
801009cc:  _nop
801009d0:  lhu v0,0xa2(s4)
801009d4:  nop
801009d8:  beq v0,zero,0x80100a28
801009dc:  _addiu v0,v0,-0x1
801009e0:  sh v0,0xa2(s4)
801009e4:  andi v0,v0,0xffff
801009e8:  bne v0,zero,0x80101024
801009ec:  _clear v0
801009f0:  jal 0x8004410c
801009f4:  _nop
801009f8:  lw v1,0x98(s4)
801009fc:  sb v0,0x5(s4)
80100a00:  lw v1,0x58(v1)
80100a04:  nop
80100a08:  lw a1,0x8(v1)
80100a0c:  sll v0,v0,0x18
80100a10:  sra a0,v0,0x18
80100a14:  clear a2
80100a18:  jal 0x800443c8
80100a1c:  _move a3,a2
80100a20:  slti a0,zero,0x409
80100a24:  clear v0
80100a28:  lhu v0,0xa0(s4)
80100a2c:  nop
80100a30:  addiu v0,v0,0x20
80100a34:  sh v0,0xa0(s4)
80100a38:  andi v0,v0,0x7fff
80100a3c:  bne v0,zero,0x80100ac8
80100a40:  _clear s1
80100a44:  li s2,0x2c
80100a48:  move s0,s4
80100a4c:  lw v1,0x98(s0)
80100a50:  nop
80100a54:  lhu v0,0xa(v1)
80100a58:  nop
80100a5c:  ori v0,v0,0x2c
80100a60:  beq v0,zero,0x80100aa0
80100a64:  _sh v0,0xa(v1)
80100a68:  lw v0,0x98(s0)
80100a6c:  nop
80100a70:  lw a0,0x30(v0)
80100a74:  jal 0x8001bddc
80100a78:  _nop
80100a7c:  lw v0,0x98(s0)
80100a80:  nop
80100a84:  lw a0,0x58(v0)
80100a88:  li a1,0x2c
80100a8c:  jal 0x8001bda0
80100a90:  _sh s2,0xa(v0)
80100a94:  lw v1,0x98(s0)
80100a98:  nop
80100a9c:  sw v0,0x30(v1)
80100aa0:  addiu s1,s1,0x1
80100aa4:  slti v0,s1,0x2
80100aa8:  bne v0,zero,0x80100a4c
80100aac:  _addiu s0,s0,0x4
80100ab0:  lb a0,0x5(s4)
80100ab4:  li v0,0x4b0
80100ab8:  jal 0x800441c8
80100abc:  _sh v0,0xa2(s4)
80100ac0:  li a1,0x1
80100ac4:  sb zero,0x5(s4)
80100ac8:  beq a1,zero,0x80101024
80100acc:  _clear v0
80100ad0:  lw a0,0x98(s4)
80100ad4:  lh a1,0xa0(s4)
80100ad8:  sltiu a0,zero,0x178
80100adc:  nop
80100ae0:  lhu a1,0xa0(s4)
80100ae4:  lw a0,0x9c(s4)
80100ae8:  ori v0,zero,0x8000
80100aec:  addu a1,a1,v0
80100af0:  sll a1,a1,0x10
80100af4:  sltiu a0,zero,0x178
80100af8:  sra a1,a1,0x10
80100afc:  lw a0,0x98(s4)
80100b00:  jal 0x800446dc
80100b04:  _addiu a0,a0,0x48
80100b08:  lw a0,0x9c(s4)
80100b0c:  move s0,v0
80100b10:  jal 0x800446dc
80100b14:  _addiu a0,a0,0x48
80100b18:  lb a0,0x5(s4)
80100b1c:  jal 0x80044574
80100b20:  _addu a1,s0,v0
80100b24:  slti a0,zero,0x409
80100b28:  clear v0
80100b2c:  lw s6,0x80(s4)
80100b30:  lui v0,0x8006
80100b34:  lw v0,0x5308(v0)
80100b38:  lw s7,0x0(s6)
80100b3c:  sll v0,v0,0x2
80100b40:  addu v0,s4,v0
80100b44:  lw s3,0x8c(v0)
80100b48:  beq s7,zero,0x80101020
80100b4c:  _addiu s5,s3,0x18
80100b50:  lw s0,0x8(s6)
80100b54:  nop
80100b58:  lh v0,0x42(s0)
80100b5c:  nop
80100b60:  bgez v0,0x80100b6c
80100b64:  _nop
80100b68:  subu v0,zero,v0
80100b6c:  slti v0,v0,0x401
80100b70:  bne v0,zero,0x80100b7c
80100b74:  _li a1,-0x8000
80100b78:  li a1,-0x7fff
80100b7c:  jal 0x8001b038
80100b80:  _move a0,s0
80100b84:  lh v1,0x42(s0)
80100b88:  li a1,-0x8000
80100b8c:  bgez v1,0x80100b98
80100b90:  _nop
80100b94:  subu v1,zero,v1
80100b98:  slti v1,v1,0x401
80100b9c:  beq v1,zero,0x80100ba8
80100ba0:  _move s2,v0
80100ba4:  li a1,-0x7fff
80100ba8:  jal 0x8001b038
80100bac:  _move a0,s0
80100bb0:  lui a0,0x8007
80100bb4:  addiu a0,a0,-0x980
80100bb8:  addiu a1,s0,0x10
80100bbc:  addiu s0,sp,0x50
80100bc0:  move a2,s0
80100bc4:  jal 0x8004cf04
80100bc8:  _move s1,v0
80100bcc:  jal 0x8004d344
80100bd0:  _move a0,s0
80100bd4:  lw a0,0x64(sp)
80100bd8:  lw v1,0x68(sp)
80100bdc:  lw v0,0x6c(sp)
80100be0:  sra a0,a0,0x8
80100be4:  sra v1,v1,0x8
80100be8:  sra v0,v0,0x8
80100bec:  ctc2 a0,0x2800
80100bf0:  ctc2 v1,0x3000
80100bf4:  ctc2 v0,0x3800
80100bf8:  lw v0,0x4(s2)
80100bfc:  lw v1,0x8(s2)
80100c00:  lw a0,0xc(s2)
80100c04:  sra v0,v0,0x8
80100c08:  andi v0,v0,0xffff
80100c0c:  sra v1,v1,0x8
80100c10:  sll v1,v1,0x10
80100c14:  addu v0,v0,v1
80100c18:  sra a0,a0,0x8
80100c1c:  mtc2 v0,0x0
80100c20:  mtc2 a0,0x800
80100c24:  nop
80100c28:  nop
80100c2c:  cop2 0x180001
80100c30:  lw v0,0x4(s1)
80100c34:  lw v1,0x8(s1)
80100c38:  lw a0,0xc(s1)
80100c3c:  sra v0,v0,0x8
80100c40:  andi v0,v0,0xffff
80100c44:  sra v1,v1,0x8
80100c48:  sll v1,v1,0x10
80100c4c:  addu v0,v0,v1
80100c50:  sra a0,a0,0x8
80100c54:  mtc2 v0,0x0
80100c58:  mtc2 a0,0x800
80100c5c:  swc2 hint9,0x30(sp)
80100c60:  swc2 hint10,0x34(sp)
80100c64:  swc2 hint11,0x38(sp)
80100c68:  nop
80100c6c:  nop
80100c70:  cop2 0x180001
80100c74:  swc2 hint9,0x40(sp)
80100c78:  swc2 hint10,0x44(sp)
80100c7c:  swc2 hint11,0x48(sp)
80100c80:  lw v0,0x80(s4)
80100c84:  nop
80100c88:  beq s6,v0,0x80100df8
80100c8c:  _nop
80100c90:  lw v0,0x18(sp)
80100c94:  nop
80100c98:  slti v0,v0,0x81
80100c9c:  beq v0,zero,0x80100cb8
80100ca0:  _nop
80100ca4:  lw v0,0x38(sp)
80100ca8:  nop
80100cac:  slti v0,v0,0x81
80100cb0:  bne v0,zero,0x80100d40
80100cb4:  _nop
80100cb8:  swc2 hint13,-0xc(s5)
80100cbc:  lw v0,0x18(sp)
80100cc0:  nop
80100cc4:  slti v0,v0,0x80
80100cc8:  bne v0,zero,0x80100ce4
80100ccc:  _addiu a0,s3,0x8
80100cd0:  lw v0,0x38(sp)
80100cd4:  nop
80100cd8:  slti v0,v0,0x80
80100cdc:  beq v0,zero,0x80100cf0
80100ce0:  _addiu a0,s3,0xc
80100ce4:  addiu a1,sp,0x10
80100ce8:  sltiu a0,zero,0xe3
80100cec:  addiu a2,sp,0x30
80100cf0:  sltiu a0,zero,0x109
80100cf4:  move a0,s3
80100cf8:  lw v1,0x38(sp)
80100cfc:  lw a0,0x18(sp)
80100d00:  nop
80100d04:  slt v0,v1,a0
80100d08:  beq v0,zero,0x80100d14
80100d0c:  _lui v0,0x8006
80100d10:  move v1,a0
80100d14:  lw v0,0x5910(v0)
80100d18:  sra v1,v1,0x3
80100d1c:  sll v1,v1,0x2
80100d20:  addu v1,v1,v0
80100d24:  lw a0,0x0(v1)
80100d28:  sll v0,s3,0x8
80100d2c:  srl v0,v0,0x8
80100d30:  sw v0,0x0(v1)
80100d34:  lui v0,0x300
80100d38:  or a0,a0,v0
80100d3c:  sw a0,0x0(s3)
80100d40:  lw v0,0x28(sp)
80100d44:  addiu s5,s5,0x10
80100d48:  slti v0,v0,0x81
80100d4c:  beq v0,zero,0x80100d68
80100d50:  _addiu s3,s3,0x10
80100d54:  lw v0,0x48(sp)
80100d58:  nop
80100d5c:  slti v0,v0,0x81
80100d60:  bne v0,zero,0x80100df0
80100d64:  _nop
80100d68:  swc2 hint14,-0xc(s5)
80100d6c:  lw v0,0x28(sp)
80100d70:  nop
80100d74:  slti v0,v0,0x80
80100d78:  bne v0,zero,0x80100d94
80100d7c:  _addiu a0,s3,0x8
80100d80:  lw v0,0x48(sp)
80100d84:  nop
80100d88:  slti v0,v0,0x80
80100d8c:  beq v0,zero,0x80100da0
80100d90:  _addiu a0,s3,0xc
80100d94:  addiu a1,sp,0x20
80100d98:  sltiu a0,zero,0xe3
80100d9c:  addiu a2,sp,0x40
80100da0:  sltiu a0,zero,0x109
80100da4:  move a0,s3
80100da8:  lw v1,0x48(sp)
80100dac:  lw a0,0x28(sp)
80100db0:  nop
80100db4:  slt v0,v1,a0
80100db8:  beq v0,zero,0x80100dc4
80100dbc:  _lui v0,0x8006
80100dc0:  move v1,a0
80100dc4:  lw v0,0x5910(v0)
80100dc8:  sra v1,v1,0x3
80100dcc:  sll v1,v1,0x2
80100dd0:  addu v1,v1,v0
80100dd4:  lw a0,0x0(v1)
80100dd8:  sll v0,s3,0x8
80100ddc:  srl v0,v0,0x8
80100de0:  sw v0,0x0(v1)
80100de4:  lui v0,0x300
80100de8:  or a0,a0,v0
80100dec:  sw a0,0x0(s3)
80100df0:  addiu s5,s5,0x10
80100df4:  addiu s3,s3,0x10
80100df8:  swc2 hint13,-0x10(s5)
80100dfc:  swc2 hint14,0x0(s5)
80100e00:  move s6,s7
80100e04:  lw t1,0x30(sp)
80100e08:  lw t2,0x34(sp)
80100e0c:  lw t3,0x38(sp)
80100e10:  lw t4,0x3c(sp)
80100e14:  sw t1,0x10(sp)
80100e18:  sw t2,0x14(sp)
80100e1c:  sw t3,0x18(sp)
80100e20:  sw t4,0x1c(sp)
80100e24:  lw t1,0x40(sp)
80100e28:  lw t2,0x44(sp)
80100e2c:  lw t3,0x48(sp)
80100e30:  lw t4,0x4c(sp)
80100e34:  sw t1,0x20(sp)
80100e38:  sw t2,0x24(sp)
80100e3c:  sw t3,0x28(sp)
80100e40:  sw t4,0x2c(sp)
80100e44:  lw s7,0x0(s7)
80100e48:  nop
80100e4c:  bne s7,zero,0x80100b50
80100e50:  _clear v0
80100e54:  slti a0,zero,0x409
80100e58:  nop
80100e5c:  jal 0x80022120
80100e60:  _move a0,s4
80100e64:  slti a0,zero,0x409
80100e68:  clear v0
80100e6c:  lw a0,0x80(s4)
80100e70:  lw v0,0x88(s4)
80100e74:  lw v1,0x8(a0)
80100e78:  lw v0,0x8(v0)
80100e7c:  lw t0,0x50(v1)
80100e80:  lw v0,0x50(v0)
80100e84:  nop
80100e88:  subu v0,v0,t0
80100e8c:  bgez v0,0x80100e98
80100e90:  _move a2,a0
80100e94:  addiu v0,v0,0xff
80100e98:  lw a0,0x0(a2)
80100e9c:  clear a1
80100ea0:  beq a0,zero,0x80100eec
80100ea4:  _sra a3,v0,0x8
80100ea8:  lw v0,0x8(a2)
80100eac:  nop
80100eb0:  lw v1,0x50(v0)
80100eb4:  nop
80100eb8:  subu v1,v1,t0
80100ebc:  sll v0,v1,0x3
80100ec0:  subu v0,v0,v1
80100ec4:  sll v0,v0,0x4
80100ec8:  div v0,a3
80100ecc:  mflo v0
80100ed0:  nop
80100ed4:  sw v0,0xc(a2)
80100ed8:  move a2,a0
80100edc:  lw a0,0x0(a0)
80100ee0:  nop
80100ee4:  bne a0,zero,0x80100ea8
80100ee8:  _addiu a1,a1,0x1
80100eec:  li a0,0x10
80100ef0:  sll s0,a1,0x1
80100ef4:  jal 0x8001178c
80100ef8:  _move a1,s0
80100efc:  li a0,0x10
80100f00:  move a1,s0
80100f04:  jal 0x8001178c
80100f08:  _sw v0,0x8c(s4)
80100f0c:  clear a0
80100f10:  blez s0,0x80100f6c
80100f14:  _sw v0,0x90(s4)
80100f18:  li a3,0x3
80100f1c:  li a2,0x40
80100f20:  move a1,s0
80100f24:  lw v0,0x8c(s4)
80100f28:  sll v1,a0,0x4
80100f2c:  addu v0,v1,v0
80100f30:  sb a3,0x3(v0)
80100f34:  lw v0,0x8c(s4)
80100f38:  nop
80100f3c:  addu v0,v1,v0
80100f40:  sb a2,0x7(v0)
80100f44:  lw v0,0x90(s4)
80100f48:  nop
80100f4c:  addu v0,v1,v0
80100f50:  sb a3,0x3(v0)
80100f54:  lw v0,0x90(s4)
80100f58:  addiu a0,a0,0x1
80100f5c:  addu v1,v1,v0
80100f60:  slt v0,a0,a1
80100f64:  bne v0,zero,0x80100f24
80100f68:  _sb a2,0x7(v1)
80100f6c:  lw a0,0x98(s4)
80100f70:  lw v1,0x0(s4)
80100f74:  clear a1
80100f78:  li v0,0x4b0
80100f7c:  sh v0,0xa2(s4)
80100f80:  ori v1,v1,0x80
80100f84:  sltiu a0,zero,0x178
80100f88:  sw v1,0x0(s4)
80100f8c:  lw a0,0x9c(s4)
80100f90:  sltiu a0,zero,0x178
80100f94:  li a1,-0x8000
80100f98:  lui a0,0x8006
80100f9c:  addiu a0,a0,0x5a50
80100fa0:  jal 0x8001ffd4
80100fa4:  _li a1,0x100
80100fa8:  lui a0,0x7f00
80100fac:  jal 0x8003d080
80100fb0:  _move a1,v0
80100fb4:  lui v1,0x8006
80100fb8:  sltu v0,zero,v0
80100fbc:  sw v0,0x5a10(v1)
80100fc0:  jal 0x80023d00
80100fc4:  _nop
80100fc8:  move a0,s4
80100fcc:  jal 0x80020890
80100fd0:  _li a1,0xf0
80100fd4:  slti a0,zero,0x409
80100fd8:  clear v0
80100fdc:  jal 0x8001d470
80100fe0:  _li a0,0xa4
80100fe4:  move s4,v0
80100fe8:  addiu v1,v0,0x84
80100fec:  sw v1,0x80(v0)
80100ff0:  addiu v1,v0,0x80
80100ff4:  sw zero,0x4(v1)
80100ff8:  slti a0,zero,0x409
80100ffc:  sw v1,0x8(v1)
80101000:  lw a0,0x8c(s4)
80101004:  jal 0x80045088
80101008:  _nop
8010100c:  lw a0,0x90(s4)
80101010:  jal 0x80045088
80101014:  _nop
80101018:  jal 0x8001fde8
8010101c:  _addiu a0,s4,0x80
80101020:  clear v0
80101024:  lw ra,0x90(sp)
80101028:  lw s7,0x8c(sp)
8010102c:  lw s6,0x88(sp)
80101030:  lw s5,0x84(sp)
80101034:  lw s4,0x80(sp)
80101038:  lw s3,0x7c(sp)
8010103c:  lw s2,0x78(sp)
80101040:  lw s1,0x74(sp)
80101044:  lw s0,0x70(sp)
80101048:  jr ra
8010104c:  _addiu sp,sp,0x98
