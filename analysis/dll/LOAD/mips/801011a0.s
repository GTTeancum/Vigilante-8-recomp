# addr: 0x801011a0  name: FUN_801011a0
801011a0:  addiu sp,sp,-0x50
801011a4:  sw s5,0x44(sp)
801011a8:  move s5,a0
801011ac:  sw s1,0x34(sp)
801011b0:  move s1,a1
801011b4:  move a0,a2
801011b8:  addiu a1,sp,0x10
801011bc:  sw s0,0x30(sp)
801011c0:  move s0,a3
801011c4:  sw ra,0x48(sp)
801011c8:  sw s4,0x40(sp)
801011cc:  sw s3,0x3c(sp)
801011d0:  jal 0x80043224
801011d4:  _sw s2,0x38(sp)
801011d8:  move a0,s0
801011dc:  addiu s0,sp,0x20
801011e0:  jal 0x8004316c
801011e4:  _move a1,s0
801011e8:  move a0,s0
801011ec:  move a1,s1
801011f0:  addiu s0,sp,0x28
801011f4:  jal 0x8004d658
801011f8:  _move a2,s0
801011fc:  lbu s2,0x28(sp)
80101200:  lui v0,0x8010
80101204:  lw a0,0x7d90(v0)
80101208:  lbu s3,0x1(s0)
8010120c:  lw s1,0x0(a0)
80101210:  lbu s4,0x2(s0)
80101214:  beq s1,zero,0x801012b4
80101218:  _li v1,0xff
8010121c:  lw s0,0x8(a0)
80101220:  addiu a1,sp,0x10
80101224:  addiu a2,sp,0x20
80101228:  sltiu a0,zero,0x3ea
8010122c:  move a0,s0
80101230:  andi a1,v0,0xffff
80101234:  beq a1,zero,0x801012a4
80101238:  _move a0,s1
8010123c:  lbu v0,0x80(s0)
80101240:  nop
80101244:  mult a1,v0
80101248:  mflo v1
8010124c:  bgez v1,0x80101258
80101250:  _nop
80101254:  addiu v1,v1,0xfff
80101258:  lbu v0,0x81(s0)
8010125c:  nop
80101260:  mult a1,v0
80101264:  sra v0,v1,0xc
80101268:  mflo a0
8010126c:  bgez a0,0x80101278
80101270:  _addu s2,s2,v0
80101274:  addiu a0,a0,0xfff
80101278:  lbu v0,0x82(s0)
8010127c:  nop
80101280:  mult a1,v0
80101284:  sra v0,a0,0xc
80101288:  mflo v1
8010128c:  bgez v1,0x80101298
80101290:  _addu s3,s3,v0
80101294:  addiu v1,v1,0xfff
80101298:  sra v0,v1,0xc
8010129c:  addu s4,s4,v0
801012a0:  move a0,s1
801012a4:  lw s1,0x0(s1)
801012a8:  nop
801012ac:  bne s1,zero,0x8010121c
801012b0:  _li v1,0xff
801012b4:  slt v0,s2,v1
801012b8:  beq v0,zero,0x801012c4
801012bc:  _nop
801012c0:  move v1,s2
801012c4:  sb v1,0x0(s5)
801012c8:  li v1,0xff
801012cc:  slt v0,s3,v1
801012d0:  beq v0,zero,0x801012dc
801012d4:  _nop
801012d8:  move v1,s3
801012dc:  sb v1,0x1(s5)
801012e0:  li v1,0xff
801012e4:  slt v0,s4,v1
801012e8:  beq v0,zero,0x801012f4
801012ec:  _nop
801012f0:  move v1,s4
801012f4:  sb v1,0x2(s5)
801012f8:  lw ra,0x48(sp)
801012fc:  lw s5,0x44(sp)
80101300:  lw s4,0x40(sp)
80101304:  lw s3,0x3c(sp)
80101308:  lw s2,0x38(sp)
8010130c:  lw s1,0x34(sp)
80101310:  lw s0,0x30(sp)
80101314:  jr ra
80101318:  _addiu sp,sp,0x50
