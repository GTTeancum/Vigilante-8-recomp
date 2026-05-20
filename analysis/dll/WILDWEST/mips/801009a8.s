# addr: 0x801009a8  name: FUN_801009a8
801009a8:  addiu sp,sp,-0x90
801009ac:  sw s1,0x84(sp)
801009b0:  move s1,a0
801009b4:  sw s0,0x80(sp)
801009b8:  move s0,a2
801009bc:  sltiu v0,a1,0x7
801009c0:  sw ra,0x8c(sp)
801009c4:  beq v0,zero,0x80100f88
801009c8:  _sw s2,0x88(sp)
801009cc:  lui v0,0x8010
801009d0:  addiu v0,v0,0x140
801009d4:  sll v1,a1,0x2
801009d8:  addu v1,v1,v0
801009dc:  lw v0,0x0(v1)
801009e0:  nop
801009e4:  jr v0
801009e8:  _nop
801009ec:  sltiu a0,zero,0x1cb
801009f0:  move a0,s1
801009f4:  beq v0,zero,0x80100a44
801009f8:  _nop
801009fc:  lw v0,0xa4(s1)
80100a00:  nop
80100a04:  beq v0,zero,0x80100a44
80100a08:  _nop
80100a0c:  lhu v0,0xc(v0)
80100a10:  nop
80100a14:  andi v0,v0,0x100
80100a18:  beq v0,zero,0x80100a44
80100a1c:  _nop
80100a20:  jal 0x8004410c
80100a24:  _nop
80100a28:  lw v1,0x58(s1)
80100a2c:  nop
80100a30:  lw a1,0x8(v1)
80100a34:  move a0,v0
80100a38:  li a2,0x6
80100a3c:  jal 0x800447e8
80100a40:  _addiu a3,s1,0x24
80100a44:  lb v0,0x8(s1)
80100a48:  nop
80100a4c:  bgez v0,0x80100ae0
80100a50:  _nop
80100a54:  lbu v1,0x9(s1)
80100a58:  lui s0,0x8006
80100a5c:  lw v0,0x5310(s0)
80100a60:  nop
80100a64:  subu v0,v0,v1
80100a68:  andi v0,v0,0x3
80100a6c:  bne v0,zero,0x80100f8c
80100a70:  _clear v0
80100a74:  lui v0,0x8007
80100a78:  lw a0,0x37d8(v0)
80100a7c:  li a1,0x21
80100a80:  li a2,0x80
80100a84:  jal 0x8001ac44
80100a88:  _li a3,0x8
80100a8c:  lw v1,0x0(v0)
80100a90:  lw a1,0x5310(s0)
80100a94:  move a0,v0
80100a98:  ori v1,v1,0x4b4
80100a9c:  sll v0,a1,0x1
80100aa0:  addu v0,v0,a1
80100aa4:  sll v0,v0,0x5
80100aa8:  sw v1,0x0(a0)
80100aac:  lw t2,0x24(s1)
80100ab0:  lw t3,0x28(s1)
80100ab4:  lw t4,0x2c(s1)
80100ab8:  sw t2,0x48(a0)
80100abc:  sw t3,0x4c(a0)
80100ac0:  sw t4,0x50(a0)
80100ac4:  sh v0,0x44(a0)
80100ac8:  lui v0,0x8010
80100acc:  addiu v0,v0,0x950
80100ad0:  jal 0x8002036c
80100ad4:  _sw v0,0x64(a0)
80100ad8:  slti a0,zero,0x3e3
80100adc:  clear v0
80100ae0:  beq s0,zero,0x80100f8c
80100ae4:  _clear v0
80100ae8:  jal 0x800449bc
80100aec:  _addiu a0,s1,0x24
80100af0:  lb a0,0x5(s1)
80100af4:  jal 0x80044574
80100af8:  _move a1,v0
80100afc:  slti a0,zero,0x3e3
80100b00:  clear v0
80100b04:  lw s2,0x0(s0)
80100b08:  nop
80100b0c:  lbu v1,0x4(s2)
80100b10:  li v0,0x7
80100b14:  bne v1,v0,0x80100bec
80100b18:  _li v0,0x2
80100b1c:  lb v0,0x8(s1)
80100b20:  nop
80100b24:  bltz v0,0x80100f8c
80100b28:  _clear v0
80100b2c:  lhu a1,0xc(s2)
80100b30:  jal 0x80022320
80100b34:  _move a0,s1
80100b38:  beq v0,zero,0x80100f88
80100b3c:  _addiu a2,s1,0x80
80100b40:  lh a0,0x14(s1)
80100b44:  lw v0,0xa8(s1)
80100b48:  lw v1,0x0(s1)
80100b4c:  mult a0,v0
80100b50:  sw zero,0xa4(s1)
80100b54:  li v0,-0x1
80100b58:  sb v0,0x8(s1)
80100b5c:  li v0,-0x101
80100b60:  and v1,v1,v0
80100b64:  mflo a0
80100b68:  bgez a0,0x80100b74
80100b6c:  _sw v1,0x0(s1)
80100b70:  addiu a0,a0,0x1f
80100b74:  sra v0,a0,0x5
80100b78:  sw v0,0x80(s1)
80100b7c:  sw zero,0x4(a2)
80100b80:  lh v1,0x20(s1)
80100b84:  lw v0,0xa8(s1)
80100b88:  nop
80100b8c:  mult v1,v0
80100b90:  mflo v0
80100b94:  bgez v0,0x80100ba0
80100b98:  _move a0,s1
80100b9c:  addiu v0,v0,0x1f
80100ba0:  li a1,0x12c
80100ba4:  sra v0,v0,0x5
80100ba8:  jal 0x80020890
80100bac:  _sw v0,0x8(a2)
80100bb0:  lb a0,0x5(s1)
80100bb4:  li v0,0x7530
80100bb8:  jal 0x800441c8
80100bbc:  _sw v0,0x94(s1)
80100bc0:  move a0,s1
80100bc4:  li a1,0x12c
80100bc8:  jal 0x80020890
80100bcc:  _sb zero,0x5(s1)
80100bd0:  lui a0,0x8006
80100bd4:  addiu a0,a0,0x5a18
80100bd8:  li a1,0x9
80100bdc:  jal 0x8002002c
80100be0:  _move a2,s1
80100be4:  slti a0,zero,0x3e3
80100be8:  clear v0
80100bec:  bne v1,v0,0x80100e5c
80100bf0:  _move a0,s1
80100bf4:  move a1,s0
80100bf8:  jal 0x8001f974
80100bfc:  _addiu a2,sp,0x10
80100c00:  addiu a0,s1,0x10
80100c04:  addiu s0,sp,0x30
80100c08:  move a1,s0
80100c0c:  jal 0x800434f8
80100c10:  _addiu a2,sp,0x48
80100c14:  lh v0,0x4c(sp)
80100c18:  nop
80100c1c:  slti v0,v0,0x801
80100c20:  bne v0,zero,0x80100f8c
80100c24:  _clear v0
80100c28:  lh v1,0x14(s1)
80100c2c:  lw v0,0xa8(s1)
80100c30:  nop
80100c34:  mult v1,v0
80100c38:  mflo v1
80100c3c:  bgez v1,0x80100c48
80100c40:  _addiu a0,sp,0x50
80100c44:  addiu v1,v1,0x1f
80100c48:  lw v0,0x80(s2)
80100c4c:  sra v1,v1,0x5
80100c50:  subu v0,v0,v1
80100c54:  sw v0,0x50(sp)
80100c58:  lw v0,0x84(s2)
80100c5c:  nop
80100c60:  sw v0,0x4(a0)
80100c64:  lh v1,0x20(s1)
80100c68:  lw v0,0xa8(s1)
80100c6c:  nop
80100c70:  mult v1,v0
80100c74:  mflo v0
80100c78:  bgez v0,0x80100c84
80100c7c:  _nop
80100c80:  addiu v0,v0,0x1f
80100c84:  lw v1,0x88(s2)
80100c88:  move a1,s0
80100c8c:  sra v0,v0,0x5
80100c90:  subu v1,v1,v0
80100c94:  jal 0x80017240
80100c98:  _sw v1,0x8(a0)
80100c9c:  srl v0,v0,0xb
80100ca0:  sll a0,v1,0x15
80100ca4:  or v0,v0,a0
80100ca8:  sra v1,v1,0xb
80100cac:  move s1,v0
80100cb0:  bgez s1,0x80100f8c
80100cb4:  _clear v0
80100cb8:  lw v0,0x40(sp)
80100cbc:  lhu v1,0x38(sp)
80100cc0:  addu v0,s1,v0
80100cc4:  move a2,v0
80100cc8:  sra a3,v0,0x1f
80100ccc:  sll v1,v1,0x10
80100cd0:  sra a0,v1,0x10
80100cd4:  sra a1,v1,0x1f
80100cd8:  subu a2,zero,a2
80100cdc:  subu a3,zero,a3
80100ce0:  sltu v0,zero,a2
80100ce4:  subu a3,a3,v0
80100ce8:  multu a0,a2
80100cec:  mfhi v1
80100cf0:  mflo v0
80100cf4:  nop
80100cf8:  nop
80100cfc:  mult a0,a3
80100d00:  mflo t0
80100d04:  nop
80100d08:  nop
80100d0c:  mult a2,a1
80100d10:  addu v1,v1,t0
80100d14:  addiu t0,sp,0x38
80100d18:  mflo a0
80100d1c:  addu v1,v1,a0
80100d20:  srl v0,v0,0xc
80100d24:  sll a0,v1,0x14
80100d28:  or v0,v0,a0
80100d2c:  sra v1,v1,0xc
80100d30:  sw v0,0x60(sp)
80100d34:  lhu v0,0x2(t0)
80100d38:  nop
80100d3c:  sll v0,v0,0x10
80100d40:  sra a0,v0,0x10
80100d44:  sra a1,v0,0x1f
80100d48:  multu a0,a2
80100d4c:  mfhi v1
80100d50:  mflo v0
80100d54:  nop
80100d58:  nop
80100d5c:  mult a0,a3
80100d60:  mflo t1
80100d64:  nop
80100d68:  nop
80100d6c:  mult a2,a1
80100d70:  addu v1,v1,t1
80100d74:  addiu a1,sp,0x60
80100d78:  mflo a0
80100d7c:  addu v1,v1,a0
80100d80:  srl v0,v0,0xc
80100d84:  sll a0,v1,0x14
80100d88:  or v0,v0,a0
80100d8c:  sra v1,v1,0xc
80100d90:  sw v0,0x4(a1)
80100d94:  lhu a0,0x4(t0)
80100d98:  nop
80100d9c:  sll a0,a0,0x10
80100da0:  sra v0,a0,0x10
80100da4:  sra v1,a0,0x1f
80100da8:  multu v0,a2
80100dac:  mfhi t1
80100db0:  mflo t0
80100db4:  nop
80100db8:  nop
80100dbc:  mult v0,a3
80100dc0:  mflo t6
80100dc4:  nop
80100dc8:  nop
80100dcc:  mult a2,v1
80100dd0:  addiu s0,sp,0x24
80100dd4:  move a0,s2
80100dd8:  move a2,s0
80100ddc:  addu t1,t1,t6
80100de0:  mflo v0
80100de4:  addu t1,t1,v0
80100de8:  srl t0,t0,0xc
80100dec:  sll v0,t1,0x14
80100df0:  or t0,t0,v0
80100df4:  sra t1,t1,0xc
80100df8:  jal 0x80017594
80100dfc:  _sw t0,0x8(a1)
80100e00:  move a0,s2
80100e04:  addiu a1,s1,0x1fff
80100e08:  sra a1,a1,0xd
80100e0c:  move a2,s0
80100e10:  jal 0x8002c958
80100e14:  _li a3,0x1
80100e18:  addiu a0,s2,0x10
80100e1c:  move a1,s0
80100e20:  addiu s0,sp,0x70
80100e24:  jal 0x80043408
80100e28:  _move a2,s0
80100e2c:  jal 0x8004410c
80100e30:  _nop
80100e34:  lui v1,0x8006
80100e38:  lw a1,0x58fc(v1)
80100e3c:  move a0,v0
80100e40:  li a2,0x5
80100e44:  jal 0x800447e8
80100e48:  _move a3,s0
80100e4c:  jal 0x80040234
80100e50:  _move a0,s0
80100e54:  slti a0,zero,0x3e3
80100e58:  clear v0
80100e5c:  lw v0,0x64(s2)
80100e60:  nop
80100e64:  beq v0,zero,0x80100f88
80100e68:  _move a0,s2
80100e6c:  li a1,0x8
80100e70:  jalr v0
80100e74:  _li a2,0x3e8
80100e78:  slti a0,zero,0x3e3
80100e7c:  clear v0
80100e80:  lb v0,0x8(s1)
80100e84:  nop
80100e88:  bgez v0,0x80100ea8
80100e8c:  _move a0,s1
80100e90:  jal 0x8003fc50
80100e94:  _move a0,s1
80100e98:  jal 0x800205f8
80100e9c:  _move a0,s1
80100ea0:  slti a0,zero,0x3e3
80100ea4:  li v0,-0x1
80100ea8:  li v0,-0x1
80100eac:  jal 0x8003fc50
80100eb0:  _sb v0,0x8(s1)
80100eb4:  lb a0,0x5(s1)
80100eb8:  jal 0x800441c8
80100ebc:  _nop
80100ec0:  move a0,s1
80100ec4:  li a1,0x12c
80100ec8:  jal 0x80020890
80100ecc:  _sb zero,0x5(a0)
80100ed0:  slti a0,zero,0x3e3
80100ed4:  li v0,-0x1
80100ed8:  slti a0,zero,0x3e3
80100edc:  li v0,0xb4
80100ee0:  sltiu a0,zero,0x1a2
80100ee4:  move a0,s1
80100ee8:  move a0,s1
80100eec:  jal 0x8001b038
80100ef0:  _ori a1,zero,0x8000
80100ef4:  move s0,v0
80100ef8:  beq s0,zero,0x80100f88
80100efc:  _lui v0,0x8007
80100f00:  lw a0,0x37d8(v0)
80100f04:  li a1,0x6
80100f08:  lui a2,0x8010
80100f0c:  jal 0x800407b4
80100f10:  _addiu a2,a2,0x130
80100f14:  move a0,s1
80100f18:  move a1,s0
80100f1c:  move s0,v0
80100f20:  jal 0x8001b2fc
80100f24:  _move a2,s0
80100f28:  jal 0x80020744
80100f2c:  _move a0,s0
80100f30:  lw v0,0x0(s1)
80100f34:  nop
80100f38:  andi v0,v0,0x4
80100f3c:  bne v0,zero,0x80100f4c
80100f40:  _nop
80100f44:  jal 0x800207c4
80100f48:  _move a0,s0
80100f4c:  jal 0x8004410c
80100f50:  _nop
80100f54:  lw v1,0x58(s1)
80100f58:  sb v0,0x5(s1)
80100f5c:  lw a1,0x8(v1)
80100f60:  sll v0,v0,0x18
80100f64:  sra a0,v0,0x18
80100f68:  clear a2
80100f6c:  jal 0x800443c8
80100f70:  _move a3,a2
80100f74:  slti a0,zero,0x3e3
80100f78:  clear v0
80100f7c:  lb a0,0x5(s1)
80100f80:  jal 0x800441c8
80100f84:  _nop
80100f88:  clear v0
80100f8c:  lw ra,0x8c(sp)
80100f90:  lw s2,0x88(sp)
80100f94:  lw s1,0x84(sp)
80100f98:  lw s0,0x80(sp)
80100f9c:  jr ra
80100fa0:  _addiu sp,sp,0x90
