# addr: 0x80048084  name: InterruptCallback
80048084:  lui v0,0x8006
80048088:  lw v0,-0x8c(v0)
8004808c:  addiu sp,sp,-0x18
80048090:  sw ra,0x10(sp)
80048094:  lw v0,0x8(v0)
80048098:  nop
8004809c:  jalr v0
800480a0:  _nop
800480a4:  lw ra,0x10(sp)
800480a8:  addiu sp,sp,0x18
800480ac:  jr ra
800480b0:  _nop
