# addr: 0x80046d04  name: SpuSetVoiceStartAddr
80046d04:  addiu sp,sp,-0x20
80046d08:  sll a0,a0,0x3
80046d0c:  sw ra,0x18(sp)
80046d10:  jal 0x80045e24
80046d14:  _ori a0,a0,0x3
80046d18:  li v0,0x1
80046d1c:  sw v0,0x14(sp)
80046d20:  j 0x80046d54
80046d24:  _sw zero,0x10(sp)
80046d28:  lw v1,0x14(sp)
80046d2c:  nop
80046d30:  sll v0,v1,0x1
80046d34:  addu v0,v0,v1
80046d38:  sll v0,v0,0x2
80046d3c:  addu v0,v0,v1
80046d40:  sw v0,0x14(sp)
80046d44:  lw v0,0x10(sp)
80046d48:  nop
80046d4c:  addiu v0,v0,0x1
80046d50:  sw v0,0x10(sp)
