# addr: 0x80048178  name: RestartCallback
80048178:  lui v0,0x8006
8004817c:  lw v0,-0x8c(v0)
80048180:  addiu sp,sp,-0x18
80048184:  sw ra,0x10(sp)
80048188:  lw v0,0x18(v0)
8004818c:  nop
80048190:  jalr v0
80048194:  _nop
80048198:  lw ra,0x10(sp)
8004819c:  addiu sp,sp,0x18
800481a0:  jr ra
800481a4:  _nop
