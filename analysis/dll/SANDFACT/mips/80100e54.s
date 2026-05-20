# addr: 0x80100e54  name: FUN_80100e54
80100e54:  addiu sp,sp,-0x60
80100e58:  sw s5,0x54(sp)
80100e5c:  move s5,a0
80100e60:  move v1,a1
80100e64:  move a1,a2
80100e68:  li v0,0x3
80100e6c:  sw ra,0x5c(sp)
80100e70:  sw s6,0x58(sp)
80100e74:  sw s4,0x50(sp)
80100e78:  sw s3,0x4c(sp)
80100e7c:  sw s2,0x48(sp)
80100e80:  sw s1,0x44(sp)
80100e84:  beq v1,v0,0x80100ea0
80100e88:  _sw s0,0x40(sp)
80100e8c:  li v0,0x8
80100e90:  beq v1,v0,0x80101090
80100e94:  _nop
80100e98:  slti a0,zero,0x426
80100e9c:  nop
80100ea0:  lw s3,0x4(a1)
80100ea4:  nop
80100ea8:  lhu v1,0x0(s3)
80100eac:  li v0,0x1
80100eb0:  bne v1,v0,0x80101074
80100eb4:  _move a0,a1
80100eb8:  lhu v0,0x2(s3)
80100ebc:  nop
80100ec0:  beq v0,zero,0x80101074
80100ec4:  _li v0,0x2
80100ec8:  lw s0,0x0(a1)
80100ecc:  nop
80100ed0:  lbu v1,0x4(s0)
80100ed4:  nop
80100ed8:  bne v1,v0,0x80101074
80100edc:  _nop
80100ee0:  lw s2,0xe0(s0)
80100ee4:  move a0,s5
80100ee8:  jal 0x8001b038
80100eec:  _ori a1,zero,0x8000
80100ef0:  jal 0x8004410c
80100ef4:  _move s6,v0
80100ef8:  lw v1,0x58(s5)
80100efc:  nop
80100f00:  lw a1,0x8(v1)
80100f04:  move a0,v0
80100f08:  li a2,0x5
80100f0c:  jal 0x800447e8
80100f10:  _addiu a3,s0,0x24
80100f14:  lb a0,0x5(s0)
80100f18:  jal 0x80044574
80100f1c:  _clear a1
80100f20:  lui a0,0x300
80100f24:  ori a0,a0,0x22
80100f28:  lui a1,0x1
80100f2c:  lw v1,0x0(s0)
80100f30:  ori a1,a1,0xc980
80100f34:  lui v0,0x8010
80100f38:  addiu v0,v0,0xc6c
80100f3c:  addiu s4,s0,0x80
80100f40:  addiu s1,s3,0x4
80100f44:  sw v0,0x64(s0)
80100f48:  sw zero,0x80(s0)
80100f4c:  or v1,v1,a0
80100f50:  sw v1,0x0(s0)
80100f54:  sw a1,0x4(s4)
80100f58:  beq s2,zero,0x80100f88
80100f5c:  _sw zero,0x8(s4)
80100f60:  lw v0,0x0(s2)
80100f64:  lui v1,0xc
80100f68:  sh zero,0x84(s2)
80100f6c:  or v0,v0,v1
80100f70:  addiu v1,s2,0x84
80100f74:  sw v0,0x0(s2)
80100f78:  li v0,-0x1c9
80100f7c:  sh v0,0x2(v1)
80100f80:  li v0,-0xbeb
80100f84:  sh v0,0x4(v1)
80100f88:  lw v0,0x4(s3)
80100f8c:  lw v1,0xc(s1)
80100f90:  nop
80100f94:  addu v0,v0,v1
80100f98:  srl v1,v0,0x1f
80100f9c:  addu v0,v0,v1
80100fa0:  sra v0,v0,0x1
80100fa4:  sw v0,0x10(sp)
80100fa8:  lw v0,0x10(s1)
80100fac:  addiu a1,sp,0x10
80100fb0:  sw v0,0x4(a1)
80100fb4:  lw v0,0x8(s1)
80100fb8:  lw v1,0x14(s1)
80100fbc:  addiu a0,s5,0x10
80100fc0:  move a2,a1
80100fc4:  addu v0,v0,v1
80100fc8:  srl v1,v0,0x1f
80100fcc:  addu v0,v0,v1
80100fd0:  sra v0,v0,0x1
80100fd4:  jal 0x80043408
80100fd8:  _sw v0,0x8(a1)
80100fdc:  lw v0,0x10(sp)
80100fe0:  lw v1,0x24(s0)
80100fe4:  nop
80100fe8:  subu v0,v0,v1
80100fec:  sll v0,v0,0x1
80100ff0:  sw v0,0x80(s0)
80100ff4:  lw v0,0x14(sp)
80100ff8:  lw v1,0x28(s0)
80100ffc:  nop
80101000:  subu v0,v0,v1
80101004:  sll v0,v0,0x1
80101008:  sw v0,0x4(s4)
8010100c:  lw v0,0x18(sp)
80101010:  lw v1,0x2c(s0)
80101014:  addiu a0,sp,0x20
80101018:  move a1,s5
8010101c:  move a2,s6
80101020:  subu v0,v0,v1
80101024:  sll v0,v0,0x1
80101028:  jal 0x8001d68c
8010102c:  _sw v0,0x8(s4)
80101030:  move a0,s0
80101034:  lw t0,0x34(sp)
80101038:  lw t1,0x38(sp)
8010103c:  lw t2,0x3c(sp)
80101040:  sw t0,0x48(a0)
80101044:  sw t1,0x4c(a0)
80101048:  sw t2,0x50(a0)
8010104c:  lwl t0,0x13(s6)
80101050:  lwr t0,0x10(s6)
80101054:  lh t1,0x14(s6)
80101058:  swl t0,0x43(a0)
8010105c:  swr t0,0x40(a0)
80101060:  sh t1,0x44(a0)
80101064:  jal 0x80020890
80101068:  _li a1,0x40
8010106c:  slti a0,zero,0x426
80101070:  nop
80101074:  lw a0,0x0(a0)
80101078:  nop
8010107c:  lbu v1,0x4(a0)
80101080:  li v0,0x7
80101084:  bne v1,v0,0x80101098
80101088:  _nop
8010108c:  lhu a1,0xc(a0)
80101090:  jal 0x80022320
80101094:  _move a0,s5
80101098:  lw ra,0x5c(sp)
8010109c:  lw s6,0x58(sp)
801010a0:  lw s5,0x54(sp)
801010a4:  lw s4,0x50(sp)
801010a8:  lw s3,0x4c(sp)
801010ac:  lw s2,0x48(sp)
801010b0:  lw s1,0x44(sp)
801010b4:  lw s0,0x40(sp)
801010b8:  clear v0
801010bc:  jr ra
801010c0:  _addiu sp,sp,0x60
