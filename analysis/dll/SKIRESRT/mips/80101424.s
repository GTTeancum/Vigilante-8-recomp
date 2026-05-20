# addr: 0x80101424  name: FUN_80101424
80101424:  addiu sp,sp,-0x18
80101428:  sw s0,0x10(sp)
8010142c:  move s0,a0
80101430:  li v0,0x1
80101434:  bne a1,v0,0x80101450
80101438:  _sw ra,0x14(sp)
8010143c:  sltiu a0,zero,0x4e4
80101440:  nop
80101444:  lui v0,0x8002
80101448:  addiu v0,v0,0x23dc
8010144c:  sw v0,0x64(s0)
80101450:  lw ra,0x14(sp)
80101454:  lw s0,0x10(sp)
80101458:  clear v0
8010145c:  jr ra
80101460:  _addiu sp,sp,0x18
