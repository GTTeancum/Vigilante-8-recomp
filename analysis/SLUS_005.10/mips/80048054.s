# addr: 0x80048054  name: ResetCallback
80048054:  lui v0,0x8006
80048058:  lw v0,-0x8c(v0)
8004805c:  addiu sp,sp,-0x18
80048060:  sw ra,0x10(sp)
80048064:  lw v0,0xc(v0)
80048068:  nop
8004806c:  jalr v0
80048070:  _nop
80048074:  lw ra,0x10(sp)
80048078:  addiu sp,sp,0x18
8004807c:  jr ra
80048080:  _nop
