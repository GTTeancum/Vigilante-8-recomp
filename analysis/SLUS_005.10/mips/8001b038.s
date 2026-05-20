# addr: 0x8001b038  name: FUN_8001b038
8001b038:  addiu sp,sp,-0x18
8001b03c:  sw ra,0x10(sp)
8001b040:  lw v0,0x58(a0)
8001b044:  lhu v1,0xa(a0)
8001b048:  lw a0,0x0(v0)
8001b04c:  andi a2,a1,0xffff
8001b050:  sll v0,v1,0x3
8001b054:  subu v0,v0,v1
8001b058:  sll v0,v0,0x2
8001b05c:  addu v0,a0,v0
8001b060:  lhu a1,0x36(v0)
8001b064:  jal 0x8001afa0
8001b068:  _nop
8001b06c:  lw ra,0x10(sp)
8001b070:  nop
8001b074:  jr ra
8001b078:  _addiu sp,sp,0x18
