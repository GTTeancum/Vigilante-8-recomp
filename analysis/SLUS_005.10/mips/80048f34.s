# addr: 0x80048f34  name: CdIntstr
80048f34:  andi a0,a0,0xff
80048f38:  sltiu v0,a0,0x7
80048f3c:  beq v0,zero,0x80048f58
80048f40:  _sll v0,a0,0x2
80048f44:  lui at,0x8006
80048f48:  addu at,at,v0
80048f4c:  lw v0,0x124(at)
80048f50:  j 0x80048f60
80048f54:  _nop
80048f58:  lui v0,0x8001
80048f5c:  addiu v0,v0,0xe74
