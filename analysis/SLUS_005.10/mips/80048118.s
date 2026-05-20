# addr: 0x80048118  name: VSyncCallbacks
80048118:  lui v0,0x8006
8004811c:  lw v0,-0x8c(v0)
80048120:  addiu sp,sp,-0x18
80048124:  sw ra,0x10(sp)
80048128:  lw v0,0x14(v0)
8004812c:  nop
80048130:  jalr v0
80048134:  _nop
80048138:  lw ra,0x10(sp)
8004813c:  addiu sp,sp,0x18
80048140:  jr ra
80048144:  _nop
