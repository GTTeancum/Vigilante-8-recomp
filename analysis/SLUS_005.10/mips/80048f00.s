# addr: 0x80048f00  name: CdComstr
80048f00:  andi a0,a0,0xff
80048f04:  sltiu v0,a0,0x1c
80048f08:  beq v0,zero,0x80048f24
80048f0c:  _sll v0,a0,0x2
80048f10:  lui at,0x8006
80048f14:  addu at,at,v0
80048f18:  lw v0,0xa4(at)
80048f1c:  j 0x80048f2c
80048f20:  _nop
80048f24:  lui v0,0x8001
80048f28:  addiu v0,v0,0xe74
