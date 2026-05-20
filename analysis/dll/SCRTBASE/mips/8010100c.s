# addr: 0x8010100c  name: FUN_8010100c
8010100c:  addiu sp,sp,-0x18
80101010:  sw ra,0x10(sp)
80101014:  beq a1,zero,0x80101030
80101018:  _move a3,a0
8010101c:  li v0,0x5
80101020:  beq a1,v0,0x801010d8
80101024:  _clear v0
80101028:  slti a0,zero,0x439
8010102c:  nop
80101030:  lw v0,0x48(a3)
80101034:  lw v1,0x88(a3)
80101038:  addiu a0,a3,0x48
8010103c:  addu v0,v0,v1
80101040:  sw v0,0x48(a3)
80101044:  lw v0,0x4(a0)
80101048:  addiu a2,a3,0x88
8010104c:  lw a1,0x4(a2)
80101050:  lw v1,0x8(a0)
80101054:  addu v0,v0,a1
80101058:  sw v0,0x4(a0)
8010105c:  lw v0,0x8(a2)
80101060:  nop
80101064:  addu v1,v1,v0
80101068:  sw v1,0x8(a0)
8010106c:  lw v0,0x88(a3)
80101070:  li v1,0xf80
80101074:  mult v0,v1
80101078:  mflo t1
8010107c:  sra v0,t1,0xc
80101080:  sw v0,0x88(a3)
80101084:  lw v0,0x4(a2)
80101088:  nop
8010108c:  mult v0,v1
80101090:  lw v0,0x8(a2)
80101094:  mflo t1
80101098:  nop
8010109c:  nop
801010a0:  mult v0,v1
801010a4:  sra v0,t1,0xc
801010a8:  sw v0,0x4(a2)
801010ac:  mflo v1
801010b0:  sra v0,v1,0xc
801010b4:  sw v0,0x8(a2)
801010b8:  lhu v0,0x44(a3)
801010bc:  lhu v1,0x96(a3)
801010c0:  move a0,a3
801010c4:  addu v0,v0,v1
801010c8:  jal 0x8001d708
801010cc:  _sh v0,0x44(a0)
801010d0:  slti a0,zero,0x439
801010d4:  clear v0
801010d8:  jal 0x800205f8
801010dc:  _move a0,a3
801010e0:  li v0,-0x1
801010e4:  lw ra,0x10(sp)
801010e8:  nop
801010ec:  jr ra
801010f0:  _addiu sp,sp,0x18
