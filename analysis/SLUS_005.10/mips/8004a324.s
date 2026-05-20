# addr: 0x8004a324  name: BIOS_OBJ_D70
8004a324:  bne v0,zero,0x8004a43c
8004a328:  _li v0,-0x1
8004a32c:  jal 0x800481a8
8004a330:  _nop
8004a334:  beq v0,zero,0x8004a3dc
8004a338:  _nop
8004a33c:  lui v0,0x8006
8004a340:  lw v0,0x344(v0)
8004a344:  nop
8004a348:  lbu v0,0x0(v0)
8004a34c:  nop
8004a350:  andi s1,v0,0x3
