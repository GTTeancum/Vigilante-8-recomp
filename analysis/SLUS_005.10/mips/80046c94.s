# addr: 0x80046c94  name: SpuSetVoicePitch
80046c94:  addiu sp,sp,-0x8
80046c98:  lui v0,0x8006
80046c9c:  lw v0,-0x122c(v0)
80046ca0:  sll a0,a0,0x4
80046ca4:  addu a0,a0,v0
80046ca8:  li v0,0x1
80046cac:  sh a1,0x4(a0)
80046cb0:  sw v0,0x4(sp)
80046cb4:  j 0x80046ce8
80046cb8:  _sw zero,0x0(sp)
80046cbc:  lw v1,0x4(sp)
80046cc0:  nop
80046cc4:  sll v0,v1,0x1
80046cc8:  addu v0,v0,v1
80046ccc:  sll v0,v0,0x2
80046cd0:  addu v0,v0,v1
80046cd4:  sw v0,0x4(sp)
80046cd8:  lw v0,0x0(sp)
80046cdc:  nop
80046ce0:  addiu v0,v0,0x1
80046ce4:  sw v0,0x0(sp)
