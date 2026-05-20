# addr: 0x8004b4a0  name: CdReadMode
8004b4a0:  lui v1,0x8006
8004b4a4:  addiu v1,v1,0x3c8
8004b4a8:  lw v0,0x0(v1)
8004b4ac:  addiu v1,v1,-0x30
8004b4b0:  jr ra
8004b4b4:  _sw a0,0x30(v1)
