# addr: 0x8004f314  name: SYS_OBJ_110
8004f314:  lui v0,0x8006
8004f318:  lbu v0,0x5026(v0)
8004f31c:  nop
8004f320:  sltiu v0,v0,0x2
8004f324:  bne v0,zero,0x8004f348
8004f328:  _nop
8004f32c:  lui a0,0x8001
8004f330:  addiu a0,a0,0x11b8
8004f334:  lui v0,0x8006
8004f338:  lw v0,0x5020(v0)
8004f33c:  nop
8004f340:  jalr v0
8004f344:  _move a1,s1
8004f348:  lui v0,0x8006
8004f34c:  lw v0,0x501c(v0)
8004f350:  nop
8004f354:  lw v0,0x34(v0)
8004f358:  nop
8004f35c:  jalr v0
8004f360:  _li a0,0x1
