# addr: 0x80100fa4  name: FUN_80100fa4
80100fa4:  addiu sp,sp,-0x18
80100fa8:  sw s0,0x10(sp)
80100fac:  move s0,a0
80100fb0:  sltiu v0,a1,0xa
80100fb4:  beq v0,zero,0x80101094
80100fb8:  _sw ra,0x14(sp)
80100fbc:  lui v0,0x8010
80100fc0:  addiu v0,v0,0x170
80100fc4:  sll v1,a1,0x2
80100fc8:  addu v1,v1,v0
80100fcc:  lw v0,0x0(v1)
80100fd0:  nop
80100fd4:  jr v0
80100fd8:  _nop
80100fdc:  lw v0,0x30(s0)
80100fe0:  nop
80100fe4:  bne v0,zero,0x80100ffc
80100fe8:  _nop
80100fec:  jal 0x800205f8
80100ff0:  _move a0,s0
80100ff4:  slti a0,zero,0x426
80100ff8:  li v0,-0x1
80100ffc:  sltiu a0,zero,0x1cb
80101000:  move a0,s0
80101004:  slti a0,zero,0x426
80101008:  clear v0
8010100c:  move a0,s0
80101010:  jal 0x8002239c
80101014:  _move a1,a2
80101018:  slti a0,zero,0x40b
8010101c:  nop
80101020:  move a0,s0
80101024:  jal 0x80022320
80101028:  _move a1,a2
8010102c:  beq v0,zero,0x80101094
80101030:  _li a1,0x9
80101034:  lui a0,0x8006
80101038:  addiu a0,a0,0x5a18
8010103c:  jal 0x8002002c
80101040:  _move a2,s0
80101044:  slti a0,zero,0x426
80101048:  clear v0
8010104c:  jal 0x8003fc50
80101050:  _move a0,s0
80101054:  slti a0,zero,0x426
80101058:  clear v0
8010105c:  lh v1,0x6(a2)
80101060:  lh v0,0x6(s0)
80101064:  nop
80101068:  bne v1,v0,0x80101098
8010106c:  _clear v0
80101070:  lw v0,0x0(s0)
80101074:  lui v1,0x1
80101078:  or v0,v0,v1
8010107c:  slti a0,zero,0x425
80101080:  sw v0,0x0(s0)
80101084:  slti a0,zero,0x426
80101088:  li v0,0xb4
8010108c:  sltiu a0,zero,0x1a2
80101090:  move a0,s0
80101094:  clear v0
80101098:  lw ra,0x14(sp)
8010109c:  lw s0,0x10(sp)
801010a0:  jr ra
801010a4:  _addiu sp,sp,0x18
