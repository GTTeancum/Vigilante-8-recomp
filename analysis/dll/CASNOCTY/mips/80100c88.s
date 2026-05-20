# addr: 0x80100c88  name: FUN_80100c88
80100c88:  addiu sp,sp,-0x20
80100c8c:  sw s1,0x14(sp)
80100c90:  move s1,a0
80100c94:  sw s2,0x18(sp)
80100c98:  move s2,a2
80100c9c:  sltiu v0,a1,0x7
80100ca0:  sw ra,0x1c(sp)
80100ca4:  beq v0,zero,0x80101a74
80100ca8:  _sw s0,0x10(sp)
80100cac:  lui v0,0x8010
80100cb0:  addiu v0,v0,0xc0
80100cb4:  sll v1,a1,0x2
80100cb8:  addu v1,v1,v0
80100cbc:  lw v0,0x0(v1)
80100cc0:  nop
80100cc4:  jr v0
80100cc8:  _nop
80100ccc:  lw v0,0x48(s1)
80100cd0:  lw v1,0x80(s1)
80100cd4:  addiu a0,s1,0x48
80100cd8:  addu v0,v0,v1
80100cdc:  sw v0,0x48(s1)
80100ce0:  lw v0,0x4(a0)
80100ce4:  addiu a2,s1,0x80
80100ce8:  lw a1,0x4(a2)
80100cec:  lw v1,0x8(a0)
80100cf0:  addu v0,v0,a1
80100cf4:  sw v0,0x4(a0)
80100cf8:  lw v0,0x8(a2)
80100cfc:  nop
80100d00:  addu v1,v1,v0
80100d04:  sw v1,0x8(a0)
80100d08:  lbu v0,0x8(s1)
80100d0c:  nop
80100d10:  addiu v0,v0,0x1
80100d14:  sll v0,v0,0x18
80100d18:  sra v1,v0,0x18
80100d1c:  sltiu v0,v1,0x7
80100d20:  beq v0,zero,0x801015b8
80100d24:  _lui v0,0x8010
80100d28:  addiu v0,v0,0xe0
80100d2c:  sll v1,v1,0x2
80100d30:  addu v1,v1,v0
80100d34:  lw v0,0x0(v1)
80100d38:  nop
80100d3c:  jr v0
80100d40:  _nop
80100d44:  jal 0x80017160
80100d48:  _nop
80100d4c:  lui a0,0x8006
80100d50:  addiu a0,a0,0x5a50
80100d54:  sll v0,v0,0x3
80100d58:  sra v0,v0,0xf
80100d5c:  jal 0x8001ffd4
80100d60:  _addiu a1,v0,-0x20
80100d64:  addiu a0,s1,0x24
80100d68:  addiu a1,v0,0x48
80100d6c:  lui a2,0x1
80100d70:  ori a2,a2,0x13a0
80100d74:  clear a3
80100d78:  jal 0x80024d30
80100d7c:  _sw v0,0x8c(s1)
80100d80:  addiu a0,s1,0x94
80100d84:  jal 0x80042e78
80100d88:  _move a1,v0
80100d8c:  li v0,0x1
80100d90:  sb v0,0x8(s1)
80100d94:  lbu v1,0x9(s1)
80100d98:  lui v0,0x8006
80100d9c:  lw v0,0x5310(v0)
80100da0:  nop
80100da4:  subu v0,v0,v1
80100da8:  andi v0,v0,0xff
80100dac:  bne v0,zero,0x80100dd8
80100db0:  _addiu a0,s1,0x24
80100db4:  lw a1,0x8c(s1)
80100db8:  lui a2,0x1
80100dbc:  ori a2,a2,0x13a0
80100dc0:  clear a3
80100dc4:  jal 0x80024d30
80100dc8:  _addiu a1,a1,0x48
80100dcc:  addiu a0,s1,0x94
80100dd0:  jal 0x80042e78
80100dd4:  _move a1,v0
80100dd8:  lh v0,0x94(s1)
80100ddc:  nop
80100de0:  bne v0,zero,0x80100e08
80100de4:  _nop
80100de8:  lw v0,0x8c(s1)
80100dec:  lw v1,0x8c(s1)
80100df0:  lw v0,0x48(v0)
80100df4:  nop
80100df8:  sw v0,0x9c(s1)
80100dfc:  lw v0,0x50(v1)
80100e00:  nop
80100e04:  sw v0,0xa0(s1)
80100e08:  move a0,s1
80100e0c:  addiu a1,s1,0x94
80100e10:  jal 0x80042f98
80100e14:  _lui a2,0x4
80100e18:  lh v1,0x94(s1)
80100e1c:  nop
80100e20:  bgez v1,0x80100e30
80100e24:  _move a0,v0
80100e28:  li v0,0x2
80100e2c:  sb v0,0x8(s1)
80100e30:  lhu v0,0xa8(s1)
80100e34:  nop
80100e38:  subu v0,a0,v0
80100e3c:  sll v0,v0,0x14
80100e40:  sra v1,v0,0x14
80100e44:  slti v0,v1,-0x8
80100e48:  bne v0,zero,0x80100e68
80100e4c:  _li a0,-0x8
80100e50:  li a0,0x8
80100e54:  move v0,a0
80100e58:  slt v0,v0,v1
80100e5c:  bne v0,zero,0x80100e68
80100e60:  _nop
80100e64:  move a0,v1
80100e68:  lhu v0,0xa8(s1)
80100e6c:  nop
80100e70:  addu v0,v0,a0
80100e74:  sll v1,v0,0x10
80100e78:  sra v1,v1,0x10
80100e7c:  bgez v1,0x80100e88
80100e80:  _sh v0,0xa8(s1)
80100e84:  addiu v1,v1,0x3f
80100e88:  lhu v0,0x42(s1)
80100e8c:  sra v1,v1,0x6
80100e90:  addu v0,v0,v1
80100e94:  sh v0,0x42(s1)
80100e98:  lb v0,0x8(s1)
80100e9c:  nop
80100ea0:  bgez v0,0x80100f7c
80100ea4:  _nop
80100ea8:  lw v0,0x80(s1)
80100eac:  nop
80100eb0:  bgez v0,0x80100edc
80100eb4:  _nop
80100eb8:  lw v0,0xa4(s1)
80100ebc:  lh v1,0x4a(s1)
80100ec0:  lh v0,0x0(v0)
80100ec4:  nop
80100ec8:  slt v1,v1,v0
80100ecc:  beq v1,zero,0x80100f00
80100ed0:  _nop
80100ed4:  slti a0,zero,0x3d6
80100ed8:  nop
80100edc:  lw v1,0xa4(s1)
80100ee0:  nop
80100ee4:  lh v0,0x0(v1)
80100ee8:  lh v1,0x4(v1)
80100eec:  lh a0,0x4a(s1)
80100ef0:  addu v0,v0,v1
80100ef4:  slt v0,v0,a0
80100ef8:  bne v0,zero,0x80100f58
80100efc:  _nop
80100f00:  lw v0,0x88(s1)
80100f04:  nop
80100f08:  bgez v0,0x80100f34
80100f0c:  _nop
80100f10:  lw v0,0xa4(s1)
80100f14:  lh v1,0x52(s1)
80100f18:  lh v0,0x2(v0)
80100f1c:  nop
80100f20:  slt v1,v1,v0
80100f24:  beq v1,zero,0x80100f7c
80100f28:  _nop
80100f2c:  slti a0,zero,0x3d6
80100f30:  nop
80100f34:  lw v1,0xa4(s1)
80100f38:  nop
80100f3c:  lh v0,0x2(v1)
80100f40:  lh v1,0x6(v1)
80100f44:  lh a0,0x52(s1)
80100f48:  addu v0,v0,v1
80100f4c:  slt v0,v0,a0
80100f50:  beq v0,zero,0x80100f7c
80100f54:  _nop
80100f58:  lhu v0,0x42(s1)
80100f5c:  lw v1,0x80(s1)
80100f60:  lw a0,0x88(s1)
80100f64:  addiu v0,v0,0x10
80100f68:  subu v1,zero,v1
80100f6c:  subu a0,zero,a0
80100f70:  sh v0,0x42(s1)
80100f74:  sw v1,0x80(s1)
80100f78:  sw a0,0x88(s1)
80100f7c:  jal 0x8001d708
80100f80:  _move a0,s1
80100f84:  lh v1,0x14(s1)
80100f88:  nop
80100f8c:  sll v0,v1,0x1
80100f90:  addu v0,v0,v1
80100f94:  sll v0,v0,0x6
80100f98:  subu v0,v0,v1
80100f9c:  sll v0,v0,0x2
80100fa0:  subu v0,v0,v1
80100fa4:  sll v0,v0,0x2
80100fa8:  subu v0,v0,v1
80100fac:  bgez v0,0x80100fb8
80100fb0:  _nop
80100fb4:  addiu v0,v0,0xfff
80100fb8:  lw v1,0x80(s1)
80100fbc:  sra v0,v0,0xc
80100fc0:  subu v0,v0,v1
80100fc4:  bgez v0,0x80100fd4
80100fc8:  _sra v1,v0,0x4
80100fcc:  addiu v0,v0,0xf
80100fd0:  sra v1,v0,0x4
80100fd4:  slti v0,v1,-0x40
80100fd8:  bne v0,zero,0x80100ff8
80100fdc:  _li a1,-0x40
80100fe0:  li a1,0x40
80100fe4:  move v0,a1
80100fe8:  slt v0,v0,v1
80100fec:  bne v0,zero,0x80100ff8
80100ff0:  _nop
80100ff4:  move a1,v1
80100ff8:  lh v0,0x20(s1)
80100ffc:  lw a0,0x80(s1)
80101000:  sll v1,v0,0x1
80101004:  addu v1,v1,v0
80101008:  sll v1,v1,0x6
8010100c:  subu v1,v1,v0
80101010:  sll v1,v1,0x2
80101014:  subu v1,v1,v0
80101018:  sll v1,v1,0x2
8010101c:  subu v0,v1,v0
80101020:  addu a0,a0,a1
80101024:  bgez v0,0x80101030
80101028:  _sw a0,0x80(s1)
8010102c:  addiu v0,v0,0xfff
80101030:  lw v1,0x88(s1)
80101034:  sra v0,v0,0xc
80101038:  subu v0,v0,v1
8010103c:  bgez v0,0x8010104c
80101040:  _sra v1,v0,0x4
80101044:  addiu v0,v0,0xf
80101048:  sra v1,v0,0x4
8010104c:  slti v0,v1,-0x40
80101050:  bne v0,zero,0x80101070
80101054:  _li a2,-0x40
80101058:  li a2,0x40
8010105c:  move v0,a2
80101060:  slt v0,v0,v1
80101064:  bne v0,zero,0x80101070
80101068:  _nop
8010106c:  move a2,v1
80101070:  lw a0,0x48(s1)
80101074:  lw v0,0x88(s1)
80101078:  lw a1,0x50(s1)
8010107c:  addu v0,v0,a2
80101080:  jal 0x80025400
80101084:  _sw v0,0x88(s1)
80101088:  lw v1,0x4c(s1)
8010108c:  lui a0,0x6
80101090:  ori a0,a0,0x4000
80101094:  addu v1,v1,a0
80101098:  subu v0,v0,v1
8010109c:  bgez v0,0x801010a8
801010a0:  _li v1,-0x2fa
801010a4:  addiu v0,v0,0xf
801010a8:  sra a0,v0,0x4
801010ac:  slt v0,v1,a0
801010b0:  beq v0,zero,0x801010bc
801010b4:  _nop
801010b8:  move v1,a0
801010bc:  lw v0,0x84(s1)
801010c0:  nop
801010c4:  subu v1,v1,v0
801010c8:  slti v0,v1,-0x40
801010cc:  bne v0,zero,0x801010ec
801010d0:  _li a0,-0x40
801010d4:  li a0,0x40
801010d8:  move v0,a0
801010dc:  slt v0,v0,v1
801010e0:  bne v0,zero,0x801010ec
801010e4:  _nop
801010e8:  move a0,v1
801010ec:  lw v0,0x84(s1)
801010f0:  nop
801010f4:  addu v0,v0,a0
801010f8:  slti a0,zero,0x56e
801010fc:  sw v0,0x84(s1)
80101100:  lw a0,0x48(s1)
80101104:  lw a1,0x50(s1)
80101108:  jal 0x80025400
8010110c:  _nop
80101110:  lw v1,0x8c(s1)
80101114:  lw a0,0x4c(s1)
80101118:  lhu a1,0x42(s1)
8010111c:  lhu v1,0x42(v1)
80101120:  lh a2,0xa8(s1)
80101124:  addiu a0,a0,0x5000
80101128:  subu s0,v0,a0
8010112c:  subu v1,v1,a1
80101130:  sll v1,v1,0x14
80101134:  sra v1,v1,0x14
80101138:  subu v1,v1,a2
8010113c:  slti v0,v1,-0x8
80101140:  bne v0,zero,0x80101160
80101144:  _li a0,-0x8
80101148:  li a0,0x8
8010114c:  move v0,a0
80101150:  slt v0,v0,v1
80101154:  bne v0,zero,0x80101160
80101158:  _nop
8010115c:  move a0,v1
80101160:  lhu v0,0xa8(s1)
80101164:  nop
80101168:  addu v0,v0,a0
8010116c:  sll v1,v0,0x10
80101170:  sra a1,v1,0x10
80101174:  bgez a1,0x80101180
80101178:  _sh v0,0xa8(s1)
8010117c:  addiu a1,a1,0x3f
80101180:  lw v0,0x80(s1)
80101184:  lhu v1,0x42(s1)
80101188:  subu a0,zero,v0
8010118c:  sra v0,a1,0x6
80101190:  addu v1,v1,v0
80101194:  bgez a0,0x801011a0
80101198:  _sh v1,0x42(s1)
8010119c:  addiu a0,a0,0xf
801011a0:  sra v1,a0,0x4
801011a4:  slti v0,v1,-0x40
801011a8:  bne v0,zero,0x801011c8
801011ac:  _li a0,-0x40
801011b0:  li a0,0x40
801011b4:  move v0,a0
801011b8:  slt v0,v0,v1
801011bc:  bne v0,zero,0x801011c8
801011c0:  _nop
801011c4:  move a0,v1
801011c8:  lw v1,0x88(s1)
801011cc:  lw v0,0x80(s1)
801011d0:  subu v1,zero,v1
801011d4:  addu v0,v0,a0
801011d8:  bgez v1,0x801011e4
801011dc:  _sw v0,0x80(s1)
801011e0:  addiu v1,v1,0xf
801011e4:  sra v1,v1,0x4
801011e8:  slti v0,v1,-0x40
801011ec:  bne v0,zero,0x8010120c
801011f0:  _li a0,-0x40
801011f4:  li a0,0x40
801011f8:  move v0,a0
801011fc:  slt v0,v0,v1
80101200:  bne v0,zero,0x8010120c
80101204:  _nop
80101208:  move a0,v1
8010120c:  lw v0,0x88(s1)
80101210:  move v1,s0
80101214:  addu v0,v0,a0
80101218:  bgez s0,0x80101224
8010121c:  _sw v0,0x88(s1)
80101220:  addiu v1,s0,0xf
80101224:  li a0,0x2fa
80101228:  sra v1,v1,0x4
8010122c:  slt v0,v1,a0
80101230:  beq v0,zero,0x8010123c
80101234:  _nop
80101238:  move a0,v1
8010123c:  lw v0,0x84(s1)
80101240:  nop
80101244:  subu v1,a0,v0
80101248:  slti v0,v1,-0x40
8010124c:  bne v0,zero,0x8010126c
80101250:  _li a1,-0x40
80101254:  li a1,0x40
80101258:  move v0,a1
8010125c:  slt v0,v0,v1
80101260:  bne v0,zero,0x8010126c
80101264:  _nop
80101268:  move a1,v1
8010126c:  lw v0,0x84(s1)
80101270:  move a0,s1
80101274:  addu v0,v0,a1
80101278:  jal 0x8001d708
8010127c:  _sw v0,0x84(s1)
80101280:  slti v0,s0,0x199
80101284:  beq v0,zero,0x801015b8
80101288:  _nop
8010128c:  lh v0,0xa8(s1)
80101290:  nop
80101294:  slti v0,v0,0x40
80101298:  beq v0,zero,0x801015b8
8010129c:  _li v0,0x3
801012a0:  sb v0,0x8(s1)
801012a4:  li v0,0x384
801012a8:  jal 0x8004410c
801012ac:  _sh v0,0xaa(s1)
801012b0:  lui v1,0x8006
801012b4:  lw a1,0x58fc(v1)
801012b8:  move a0,v0
801012bc:  li a2,0x14
801012c0:  jal 0x8004483c
801012c4:  _addiu a3,s1,0x48
801012c8:  slti a0,zero,0x56e
801012cc:  nop
801012d0:  lhu v0,0xaa(s1)
801012d4:  li v1,-0x1
801012d8:  addiu v0,v0,-0x1
801012dc:  sh v0,0xaa(s1)
801012e0:  sll v0,v0,0x10
801012e4:  sra v0,v0,0x10
801012e8:  bne v0,v1,0x801015b8
801012ec:  _nop
801012f0:  sh zero,0xaa(s1)
801012f4:  slti a0,zero,0x56e
801012f8:  sb zero,0x8(s1)
801012fc:  lw v0,0x80(s1)
80101300:  nop
80101304:  bgez v0,0x80101330
80101308:  _nop
8010130c:  lw v0,0xa4(s1)
80101310:  lh v1,0x4a(s1)
80101314:  lh v0,0x0(v0)
80101318:  nop
8010131c:  slt v1,v1,v0
80101320:  beq v1,zero,0x80101354
80101324:  _nop
80101328:  slti a0,zero,0x4ec
8010132c:  sw zero,0x88(s1)
80101330:  lw v1,0xa4(s1)
80101334:  nop
80101338:  lh v0,0x0(v1)
8010133c:  lh v1,0x4(v1)
80101340:  lh a0,0x4a(s1)
80101344:  addu v0,v0,v1
80101348:  slt v0,v0,a0
8010134c:  bne v0,zero,0x801013ac
80101350:  _nop
80101354:  lw v0,0x88(s1)
80101358:  nop
8010135c:  bgez v0,0x80101388
80101360:  _nop
80101364:  lw v0,0xa4(s1)
80101368:  lh v1,0x52(s1)
8010136c:  lh v0,0x2(v0)
80101370:  nop
80101374:  slt v1,v1,v0
80101378:  beq v1,zero,0x801013b4
8010137c:  _nop
80101380:  slti a0,zero,0x4ec
80101384:  sw zero,0x88(s1)
80101388:  lw v1,0xa4(s1)
8010138c:  nop
80101390:  lh v0,0x2(v1)
80101394:  lh v1,0x6(v1)
80101398:  lh a0,0x52(s1)
8010139c:  addu v0,v0,v1
801013a0:  slt v0,v0,a0
801013a4:  beq v0,zero,0x801013b4
801013a8:  _nop
801013ac:  sw zero,0x88(s1)
801013b0:  sw zero,0x80(s1)
801013b4:  lh v0,0x40(s1)
801013b8:  lhu v1,0x40(s1)
801013bc:  slti v0,v0,-0x1ff
801013c0:  bne v0,zero,0x801013cc
801013c4:  _addiu v0,v1,-0x4
801013c8:  sh v0,0x40(s1)
801013cc:  jal 0x8001d708
801013d0:  _move a0,s1
801013d4:  lh v1,0x14(s1)
801013d8:  nop
801013dc:  sll v0,v1,0x1
801013e0:  addu v0,v0,v1
801013e4:  sll v0,v0,0x6
801013e8:  subu v0,v0,v1
801013ec:  sll v0,v0,0x2
801013f0:  subu v0,v0,v1
801013f4:  sll v0,v0,0x2
801013f8:  subu v0,v0,v1
801013fc:  bgez v0,0x80101408
80101400:  _nop
80101404:  addiu v0,v0,0xfff
80101408:  lw v1,0x80(s1)
8010140c:  sra v0,v0,0xc
80101410:  subu v0,v0,v1
80101414:  bgez v0,0x80101424
80101418:  _sra v1,v0,0x4
8010141c:  addiu v0,v0,0xf
80101420:  sra v1,v0,0x4
80101424:  slti v0,v1,-0x40
80101428:  bne v0,zero,0x80101448
8010142c:  _li a1,-0x40
80101430:  li a1,0x40
80101434:  move v0,a1
80101438:  slt v0,v0,v1
8010143c:  bne v0,zero,0x80101448
80101440:  _nop
80101444:  move a1,v1
80101448:  lh v0,0x1a(s1)
8010144c:  lw a0,0x80(s1)
80101450:  sll v1,v0,0x1
80101454:  addu v1,v1,v0
80101458:  sll v1,v1,0x6
8010145c:  subu v1,v1,v0
80101460:  sll v1,v1,0x2
80101464:  subu v1,v1,v0
80101468:  sll v1,v1,0x2
8010146c:  subu v0,v1,v0
80101470:  addu a0,a0,a1
80101474:  bgez v0,0x80101480
80101478:  _sw a0,0x80(s1)
8010147c:  addiu v0,v0,0xfff
80101480:  lw v1,0x84(s1)
80101484:  sra v0,v0,0xc
80101488:  subu v0,v0,v1
8010148c:  bgez v0,0x8010149c
80101490:  _sra v1,v0,0x4
80101494:  addiu v0,v0,0xf
80101498:  sra v1,v0,0x4
8010149c:  slti v0,v1,-0x40
801014a0:  bne v0,zero,0x801014c0
801014a4:  _li a1,-0x40
801014a8:  li a1,0x40
801014ac:  move v0,a1
801014b0:  slt v0,v0,v1
801014b4:  bne v0,zero,0x801014c0
801014b8:  _nop
801014bc:  move a1,v1
801014c0:  lh v0,0x20(s1)
801014c4:  lw a0,0x84(s1)
801014c8:  sll v1,v0,0x1
801014cc:  addu v1,v1,v0
801014d0:  sll v1,v1,0x6
801014d4:  subu v1,v1,v0
801014d8:  sll v1,v1,0x2
801014dc:  subu v1,v1,v0
801014e0:  sll v1,v1,0x2
801014e4:  subu v0,v1,v0
801014e8:  addu a0,a0,a1
801014ec:  bgez v0,0x801014f8
801014f0:  _sw a0,0x84(s1)
801014f4:  addiu v0,v0,0xfff
801014f8:  lw v1,0x88(s1)
801014fc:  sra v0,v0,0xc
80101500:  subu v0,v0,v1
80101504:  bgez v0,0x80101514
80101508:  _sra v1,v0,0x4
8010150c:  addiu v0,v0,0xf
80101510:  sra v1,v0,0x4
80101514:  slti v0,v1,-0x40
80101518:  bne v0,zero,0x80101538
8010151c:  _li a2,-0x40
80101520:  li a2,0x40
80101524:  move v0,a2
80101528:  slt v0,v0,v1
8010152c:  bne v0,zero,0x80101538
80101530:  _nop
80101534:  move a2,v1
80101538:  lw a0,0x48(s1)
8010153c:  lw v0,0x88(s1)
80101540:  lw a1,0x50(s1)
80101544:  addu v0,v0,a2
80101548:  jal 0x80025400
8010154c:  _sw v0,0x88(s1)
80101550:  lw v1,0x4c(s1)
80101554:  nop
80101558:  slt v0,v0,v1
8010155c:  beq v0,zero,0x801015b8
80101560:  _nop
80101564:  lw v0,0x0(s1)
80101568:  nop
8010156c:  andi v0,v0,0x8000
80101570:  bne v0,zero,0x801015b8
80101574:  _nop
80101578:  jal 0x8003fc50
8010157c:  _move a0,s1
80101580:  li v0,0x5
80101584:  slti a0,zero,0x56e
80101588:  sb v0,0x8(s1)
8010158c:  lw v0,0x0(s1)
80101590:  nop
80101594:  andi v0,v0,0x8000
80101598:  bne v0,zero,0x801015b8
8010159c:  _nop
801015a0:  lb a0,0x5(s1)
801015a4:  jal 0x800441c8
801015a8:  _nop
801015ac:  move a0,s1
801015b0:  jal 0x800205f8
801015b4:  _sb zero,0x5(s1)
801015b8:  beq s2,zero,0x80101a78
801015bc:  _clear v0
801015c0:  lw s0,0x38(s1)
801015c4:  nop
801015c8:  beq s0,zero,0x80101658
801015cc:  _sll s2,s2,0x8
801015d0:  lh v1,0x6(s0)
801015d4:  nop
801015d8:  bne v1,zero,0x801015f4
801015dc:  _li v0,0x1
801015e0:  lhu v0,0x44(s0)
801015e4:  move a0,s0
801015e8:  addu v0,v0,s2
801015ec:  slti a0,zero,0x590
801015f0:  sh v0,0x44(s0)
801015f4:  bne v1,v0,0x80101648
801015f8:  _li v0,0x3
801015fc:  lb v1,0x8(s1)
80101600:  nop
80101604:  bne v1,v0,0x80101628
80101608:  _nop
8010160c:  lh v0,0x40(s0)
80101610:  lhu v1,0x40(s0)
80101614:  slti v0,v0,0x355
80101618:  beq v0,zero,0x80101648
8010161c:  _addiu v0,v1,0x20
80101620:  slti a0,zero,0x58f
80101624:  sh v0,0x40(s0)
80101628:  lh v0,0x40(s0)
8010162c:  lhu v1,0x40(s0)
80101630:  beq v0,zero,0x80101648
80101634:  _addiu v0,v1,-0x20
80101638:  sh v0,0x40(s0)
8010163c:  move a0,s0
80101640:  jal 0x8001d708
80101644:  _nop
80101648:  lw s0,0x34(s0)
8010164c:  nop
80101650:  bne s0,zero,0x801015d0
80101654:  _nop
80101658:  jal 0x800449bc
8010165c:  _addiu a0,s1,0x48
80101660:  lb a0,0x5(s1)
80101664:  jal 0x80044574
80101668:  _move a1,v0
8010166c:  slti a0,zero,0x69e
80101670:  clear v0
80101674:  lw v0,0x0(s1)
80101678:  li v1,-0x21
8010167c:  and v0,v0,v1
80101680:  slti a0,zero,0x69d
80101684:  sw v0,0x0(s1)
80101688:  lw s0,0x0(s2)
8010168c:  nop
80101690:  lbu v1,0x4(s0)
80101694:  li v0,0x2
80101698:  bne v1,v0,0x8010185c
8010169c:  _nop
801016a0:  lh v0,0x6(s0)
801016a4:  nop
801016a8:  bgez v0,0x8010185c
801016ac:  _li v0,0x3
801016b0:  lb v1,0x8(s1)
801016b4:  nop
801016b8:  bne v1,v0,0x80101868
801016bc:  _slti v0,v1,0x4
801016c0:  jal 0x80016c88
801016c4:  _addiu a0,s0,0x10
801016c8:  lhu v1,0x42(s1)
801016cc:  nop
801016d0:  subu v0,v0,v1
801016d4:  sll v0,v0,0x14
801016d8:  sra v0,v0,0x10
801016dc:  bgez v0,0x801016e8
801016e0:  _nop
801016e4:  subu v0,zero,v0
801016e8:  slti v0,v0,0x1000
801016ec:  beq v0,zero,0x8010185c
801016f0:  _lui a0,0x100
801016f4:  lw s2,0xe0(s0)
801016f8:  sw s0,0x90(s1)
801016fc:  lw v1,0x0(s0)
80101700:  ori a0,a0,0x20
80101704:  lui v0,0x8010
80101708:  addiu v0,v0,0x6d4
8010170c:  sw v0,0x64(s0)
80101710:  li v0,-0x9
80101714:  sw s1,0x78(s0)
80101718:  or v1,v1,a0
8010171c:  and v1,v1,v0
80101720:  sw v1,0x0(s0)
80101724:  lw v0,0x48(s1)
80101728:  lw v1,0x24(s0)
8010172c:  nop
80101730:  subu v0,v0,v1
80101734:  sll v0,v0,0x2
80101738:  sw v0,0x80(s0)
8010173c:  lw v0,0x4c(s1)
80101740:  lw v1,0x28(s0)
80101744:  addiu a0,s0,0x80
80101748:  subu v0,v0,v1
8010174c:  sll v0,v0,0x2
80101750:  sw v0,0x4(a0)
80101754:  lw v0,0x50(s1)
80101758:  lw v1,0x2c(s0)
8010175c:  nop
80101760:  subu v0,v0,v1
80101764:  sll v0,v0,0x2
80101768:  sw v0,0x8(a0)
8010176c:  lb a0,0x5(s0)
80101770:  clear a1
80101774:  jal 0x80044574
80101778:  _sh zero,0xa4(s0)
8010177c:  move a0,s0
80101780:  jal 0x80020890
80101784:  _li a1,0x20
80101788:  beq s2,zero,0x8010183c
8010178c:  _nop
80101790:  jal 0x800116f4
80101794:  _li a0,0x30
80101798:  li v1,0x78
8010179c:  lw t2,0x48(s2)
801017a0:  lw t3,0x4c(s2)
801017a4:  lw t4,0x50(s2)
801017a8:  sw t2,0x0(v0)
801017ac:  sw t3,0x4(v0)
801017b0:  sw t4,0x8(v0)
801017b4:  sw v1,0xc(v0)
801017b8:  lh a0,0x14(s1)
801017bc:  lw a1,0x24(s1)
801017c0:  sll v1,a0,0x1
801017c4:  addu v1,v1,a0
801017c8:  sll v1,v1,0x3
801017cc:  addu v1,v1,a0
801017d0:  sll v1,v1,0x2
801017d4:  subu a1,a1,v1
801017d8:  sw a1,0x10(v0)
801017dc:  lw v1,0x28(s1)
801017e0:  addiu a3,v0,0x10
801017e4:  lui a0,0xfffc
801017e8:  ori a0,a0,0xe000
801017ec:  addu v1,v1,a0
801017f0:  sw v1,0x4(a3)
801017f4:  lh v1,0x20(s1)
801017f8:  move a1,v0
801017fc:  move a0,s2
80101800:  lw a2,0x2c(s1)
80101804:  sll v0,v1,0x1
80101808:  addu v0,v0,v1
8010180c:  sll v0,v0,0x3
80101810:  addu v0,v0,v1
80101814:  sll v0,v0,0x2
80101818:  subu a2,a2,v0
8010181c:  sw a2,0x8(a3)
80101820:  jal 0x8003dbb0
80101824:  _sw zero,0x1c(a1)
80101828:  lui v0,0xf
8010182c:  ori v0,v0,0xa000
80101830:  sw v0,0x94(s2)
80101834:  sh zero,0xaa(s1)
80101838:  sb zero,0x8(s1)
8010183c:  jal 0x8004410c
80101840:  _nop
80101844:  lw v1,0x58(s1)
80101848:  nop
8010184c:  lw a1,0x8(v1)
80101850:  move a0,v0
80101854:  slti a0,zero,0x670
80101858:  clear a2
8010185c:  lb v1,0x8(s1)
80101860:  nop
80101864:  slti v0,v1,0x4
80101868:  beq v0,zero,0x801019d0
8010186c:  _li v0,0x7
80101870:  lbu v1,0x4(s0)
80101874:  nop
80101878:  bne v1,v0,0x801018b8
8010187c:  _li v0,0x3
80101880:  lhu a1,0xc(s0)
80101884:  jal 0x80022320
80101888:  _move a0,s1
8010188c:  beq v0,zero,0x80101a74
80101890:  _li v0,0x4
80101894:  sb v0,0x8(s1)
80101898:  move a0,s1
8010189c:  sltiu a0,zero,0x2f4
801018a0:  ori a1,zero,0x8000
801018a4:  move a0,s1
801018a8:  sltiu a0,zero,0x2f4
801018ac:  ori a1,zero,0x8001
801018b0:  slti a0,zero,0x69e
801018b4:  clear v0
801018b8:  beq v1,v0,0x80101a74
801018bc:  _move a0,s1
801018c0:  jal 0x8001f5a0
801018c4:  _move a1,s2
801018c8:  lh t0,0x20(s2)
801018cc:  lw t1,0x80(s1)
801018d0:  nop
801018d4:  mult t1,t0
801018d8:  addiu a1,s2,0x20
801018dc:  lh v1,0x2(a1)
801018e0:  addiu a0,s1,0x80
801018e4:  lw v0,0x4(a0)
801018e8:  mflo a3
801018ec:  nop
801018f0:  nop
801018f4:  mult v0,v1
801018f8:  lh v1,0x4(a1)
801018fc:  lw v0,0x8(a0)
80101900:  mflo a2
80101904:  nop
80101908:  nop
8010190c:  mult v0,v1
80101910:  addu v0,a3,a2
80101914:  mflo v1
80101918:  addu v0,v0,v1
8010191c:  bgez v0,0x8010192c
80101920:  _sra a0,v0,0xb
80101924:  addiu v0,v0,0x7ff
80101928:  sra a0,v0,0xb
8010192c:  bgez a0,0x80101a78
80101930:  _clear v0
80101934:  mult a0,t0
80101938:  mflo v0
8010193c:  bgez v0,0x80101948
80101940:  _nop
80101944:  addiu v0,v0,0xfff
80101948:  sra v0,v0,0xc
8010194c:  subu v0,t1,v0
80101950:  sw v0,0x80(s1)
80101954:  lh v0,0x22(s2)
80101958:  nop
8010195c:  mult a0,v0
80101960:  mflo v1
80101964:  bgez v1,0x80101970
80101968:  _nop
8010196c:  addiu v1,v1,0xfff
80101970:  lw v0,0x84(s1)
80101974:  sra v1,v1,0xc
80101978:  subu v0,v0,v1
8010197c:  sw v0,0x84(s1)
80101980:  lh v0,0x24(s2)
80101984:  nop
80101988:  mult a0,v0
8010198c:  mflo v1
80101990:  bgez v1,0x8010199c
80101994:  _nop
80101998:  addiu v1,v1,0xfff
8010199c:  lw v0,0x88(s1)
801019a0:  sra v1,v1,0xc
801019a4:  subu v0,v0,v1
801019a8:  jal 0x8004410c
801019ac:  _sw v0,0x88(s1)
801019b0:  lui v1,0x8006
801019b4:  lw a1,0x58fc(v1)
801019b8:  move a0,v0
801019bc:  li a2,0x7
801019c0:  jal 0x8004483c
801019c4:  _addiu a3,s1,0x48
801019c8:  slti a0,zero,0x69e
801019cc:  clear v0
801019d0:  slti v0,v1,0x5
801019d4:  beq v0,zero,0x80101a78
801019d8:  _clear v0
801019dc:  lbu v0,0x4(s0)
801019e0:  nop
801019e4:  bne v0,zero,0x80101a78
801019e8:  _clear v0
801019ec:  lw v0,0x0(s1)
801019f0:  nop
801019f4:  andi v0,v0,0x8000
801019f8:  bne v0,zero,0x80101a78
801019fc:  _clear v0
80101a00:  jal 0x8003fc50
80101a04:  _move a0,s1
80101a08:  li v0,0x5
80101a0c:  slti a0,zero,0x69d
80101a10:  sb v0,0x8(s1)
80101a14:  slti a0,zero,0x69e
80101a18:  li v0,0xac
80101a1c:  clear a0
80101a20:  li v0,0x84
80101a24:  jal 0x8001fd9c
80101a28:  _sw v0,0x0(s1)
80101a2c:  addiu v0,v0,0xc
80101a30:  jal 0x8004410c
80101a34:  _sw v0,0xa4(s1)
80101a38:  lw v1,0x58(s1)
80101a3c:  sb v0,0x5(s1)
80101a40:  lw a1,0x8(v1)
80101a44:  sll v0,v0,0x18
80101a48:  sra a0,v0,0x18
80101a4c:  li a2,0x4
80101a50:  jal 0x800443c8
80101a54:  _clear a3
80101a58:  slti a0,zero,0x69e
80101a5c:  clear v0
80101a60:  jal 0x80042f5c
80101a64:  _addiu a0,s1,0x94
80101a68:  lb a0,0x5(s1)
80101a6c:  jal 0x800441c8
80101a70:  _nop
80101a74:  clear v0
80101a78:  lw ra,0x1c(sp)
80101a7c:  lw s2,0x18(sp)
80101a80:  lw s1,0x14(sp)
80101a84:  lw s0,0x10(sp)
80101a88:  jr ra
80101a8c:  _addiu sp,sp,0x20
