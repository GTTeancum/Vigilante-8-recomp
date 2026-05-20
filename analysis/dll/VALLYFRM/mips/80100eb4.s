# addr: 0x80100eb4  name: FUN_80100eb4
80100eb4:  addiu sp,sp,-0x20
80100eb8:  sw s1,0x14(sp)
80100ebc:  move s1,a0
80100ec0:  sw s2,0x18(sp)
80100ec4:  move s2,a2
80100ec8:  li a2,0x1
80100ecc:  sw ra,0x1c(sp)
80100ed0:  beq a1,a2,0x801011c4
80100ed4:  _sw s0,0x10(sp)
80100ed8:  beq a1,zero,0x80100ef8
80100edc:  _li v0,0x3
80100ee0:  beq a1,v0,0x801010f4
80100ee4:  _li v0,0x4
80100ee8:  beq a1,v0,0x801011f8
80100eec:  _clear v0
80100ef0:  slti a0,zero,0x482
80100ef4:  nop
80100ef8:  lb v1,0x8(s1)
80100efc:  nop
80100f00:  beq v1,a2,0x80101004
80100f04:  _slti v0,v1,0x2
80100f08:  beq v0,zero,0x80100f20
80100f0c:  _li v0,0x2
80100f10:  beq v1,zero,0x80100f30
80100f14:  _lui v0,0x4d3
80100f18:  slti a0,zero,0x415
80100f1c:  nop
80100f20:  beq v1,v0,0x80101028
80100f24:  _lui v0,0x392
80100f28:  slti a0,zero,0x415
80100f2c:  nop
80100f30:  lw a0,0x2c(s1)
80100f34:  ori v0,v0,0xffff
80100f38:  addiu v1,a0,-0x1dcd
80100f3c:  slt v0,v0,v1
80100f40:  bne v0,zero,0x80100f58
80100f44:  _sw v1,0x2c(s1)
80100f48:  lui v0,0x4d4
80100f4c:  slt v0,v0,a0
80100f50:  bne v0,zero,0x80100f78
80100f54:  _nop
80100f58:  lui v0,0x4b4
80100f5c:  ori v0,v0,0xffff
80100f60:  slt v0,v0,v1
80100f64:  bne v0,zero,0x80100fd8
80100f68:  _lui v0,0x4b5
80100f6c:  slt v0,v0,a0
80100f70:  beq v0,zero,0x80100fd8
80100f74:  _nop
80100f78:  lw t0,0x24(s1)
80100f7c:  lw t1,0x28(s1)
80100f80:  lw t2,0x2c(s1)
80100f84:  sw t0,0x48(s1)
80100f88:  sw t1,0x4c(s1)
80100f8c:  sw t2,0x50(s1)
80100f90:  jal 0x80021c6c
80100f94:  _move a0,s1
80100f98:  move a0,v0
80100f9c:  li v0,0x1
80100fa0:  sb v0,0x8(a0)
80100fa4:  li v0,0x400
80100fa8:  jal 0x8002036c
80100fac:  _sh v0,0x42(a0)
80100fb0:  jal 0x80021c6c
80100fb4:  _move a0,s1
80100fb8:  move a0,v0
80100fbc:  li v0,0x2
80100fc0:  sb v0,0x8(a0)
80100fc4:  li v0,-0x400
80100fc8:  jal 0x8002036c
80100fcc:  _sh v0,0x42(a0)
80100fd0:  slti a0,zero,0x415
80100fd4:  nop
80100fd8:  lw v1,0x2c(s1)
80100fdc:  lui v0,0x4a8
80100fe0:  ori v0,v0,0xffff
80100fe4:  slt v0,v0,v1
80100fe8:  bne v0,zero,0x80101054
80100fec:  _lui v1,0x8010
80100ff0:  lw v0,0x1308(v1)
80100ff4:  move a0,s1
80100ff8:  addiu v0,v0,-0x1
80100ffc:  slti a0,zero,0x411
80101000:  sw v0,0x1308(v1)
80101004:  lw v0,0x24(s1)
80101008:  lui v1,0x33d
8010100c:  ori v1,v1,0xffff
80101010:  addiu v0,v0,-0x1dcd
80101014:  slt v1,v1,v0
80101018:  bne v1,zero,0x80101054
8010101c:  _sw v0,0x24(s1)
80101020:  slti a0,zero,0x411
80101024:  move a0,s1
80101028:  lw v1,0x24(s1)
8010102c:  nop
80101030:  addiu v1,v1,0x1dcd
80101034:  slt v0,v0,v1
80101038:  beq v0,zero,0x80101054
8010103c:  _sw v1,0x24(s1)
80101040:  move a0,s1
80101044:  jal 0x800205f8
80101048:  _nop
8010104c:  slti a0,zero,0x482
80101050:  li v0,-0x1
80101054:  lbu v1,0x9(s1)
80101058:  lui v0,0x8006
8010105c:  lw v0,0x5310(v0)
80101060:  nop
80101064:  subu v0,v0,v1
80101068:  andi v0,v0,0x3
8010106c:  bne v0,zero,0x801010d0
80101070:  _lui v0,0x8007
80101074:  lw a0,0x37d8(v0)
80101078:  li a1,0x1f
8010107c:  li a2,0x80
80101080:  jal 0x8001ac44
80101084:  _li a3,0x8
80101088:  move s0,v0
8010108c:  li v0,0x10
80101090:  jal 0x80017160
80101094:  _sw v0,0x0(s0)
80101098:  lui v1,0xfffe
8010109c:  ori v1,v1,0xc000
801010a0:  move a0,s1
801010a4:  move a1,s0
801010a8:  andi v0,v0,0xff
801010ac:  addiu v0,v0,-0x80
801010b0:  sll v0,v0,0xa
801010b4:  sw v0,0x24(a1)
801010b8:  lui v0,0x8010
801010bc:  addiu v0,v0,0xe70
801010c0:  sw zero,0x28(a1)
801010c4:  sw v1,0x2c(a1)
801010c8:  jal 0x8001d4f0
801010cc:  _sw v0,0x64(a1)
801010d0:  beq s2,zero,0x80101208
801010d4:  _clear v0
801010d8:  jal 0x800446dc
801010dc:  _addiu a0,s1,0x24
801010e0:  lb a0,0x5(s1)
801010e4:  jal 0x80044574
801010e8:  _move a1,v0
801010ec:  slti a0,zero,0x482
801010f0:  clear v0
801010f4:  lw a0,0x0(s2)
801010f8:  nop
801010fc:  lbu a1,0x4(a0)
80101100:  li v0,0x2
80101104:  bne a1,v0,0x80101204
80101108:  _nop
8010110c:  lb v1,0x8(s1)
80101110:  nop
80101114:  beq v1,a2,0x8010115c
80101118:  _move s0,a0
8010111c:  slt v0,v1,v0
80101120:  beq v0,zero,0x80101138
80101124:  _nop
80101128:  beq v1,zero,0x80101148
8010112c:  _lui v1,0xfffe
80101130:  slti a0,zero,0x45f
80101134:  nop
80101138:  beq v1,a1,0x8010116c
8010113c:  _lui v1,0x1
80101140:  slti a0,zero,0x45f
80101144:  nop
80101148:  lw v0,0x88(s0)
8010114c:  ori v1,v1,0xcf00
80101150:  addu v0,v0,v1
80101154:  slti a0,zero,0x45f
80101158:  sw v0,0x88(s0)
8010115c:  lw v0,0x80(s0)
80101160:  lui v1,0xfffe
80101164:  slti a0,zero,0x45d
80101168:  ori v1,v1,0xcf00
8010116c:  lw v0,0x80(s0)
80101170:  ori v1,v1,0x3100
80101174:  addu v0,v0,v1
80101178:  sw v0,0x80(s0)
8010117c:  jal 0x80017160
80101180:  _nop
80101184:  andi v0,v0,0x1f
80101188:  bne v0,zero,0x80101198
8010118c:  _nop
80101190:  jal 0x8002c4bc
80101194:  _move a0,s0
80101198:  jal 0x8004410c
8010119c:  _nop
801011a0:  lw v1,0x58(s1)
801011a4:  nop
801011a8:  lw a1,0x8(v1)
801011ac:  move a0,v0
801011b0:  li a2,0x5
801011b4:  jal 0x8004483c
801011b8:  _addiu a3,s1,0x24
801011bc:  slti a0,zero,0x482
801011c0:  clear v0
801011c4:  li v0,0x84
801011c8:  jal 0x8004410c
801011cc:  _sw v0,0x0(s1)
801011d0:  lw v1,0x58(s1)
801011d4:  sb v0,0x5(s1)
801011d8:  lw a1,0x8(v1)
801011dc:  sll v0,v0,0x18
801011e0:  sra a0,v0,0x18
801011e4:  li a2,0x3
801011e8:  jal 0x800443c8
801011ec:  _clear a3
801011f0:  slti a0,zero,0x482
801011f4:  clear v0
801011f8:  lb a0,0x5(s1)
801011fc:  jal 0x800441c8
80101200:  _nop
80101204:  clear v0
80101208:  lw ra,0x1c(sp)
8010120c:  lw s2,0x18(sp)
80101210:  lw s1,0x14(sp)
80101214:  lw s0,0x10(sp)
80101218:  jr ra
8010121c:  _addiu sp,sp,0x20
