# addr: 0x80046c14  name: SpuSetVoiceVolume
80046c14:  addiu sp,sp,-0x8
80046c18:  andi a1,a1,0x7fff
80046c1c:  sll a0,a0,0x4
80046c20:  lui v0,0x8006
80046c24:  lw v0,-0x122c(v0)
80046c28:  andi a2,a2,0x7fff
80046c2c:  addu a0,a0,v0
80046c30:  li v0,0x1
80046c34:  sh a1,0x0(a0)
80046c38:  sh a2,0x2(a0)
80046c3c:  sw v0,0x4(sp)
80046c40:  j 0x80046c74
80046c44:  _sw zero,0x0(sp)
80046c48:  lw v1,0x4(sp)
80046c4c:  nop
80046c50:  sll v0,v1,0x1
80046c54:  addu v0,v0,v1
80046c58:  sll v0,v0,0x2
80046c5c:  addu v0,v0,v1
80046c60:  sw v0,0x4(sp)
80046c64:  lw v0,0x0(sp)
80046c68:  nop
80046c6c:  addiu v0,v0,0x1
80046c70:  sw v0,0x0(sp)
