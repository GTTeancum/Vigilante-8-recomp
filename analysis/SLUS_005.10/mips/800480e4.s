# addr: 0x800480e4  name: VSyncCallback
800480e4:  addiu sp,sp,-0x18
800480e8:  lui v0,0x8006
800480ec:  lw v0,-0x8c(v0)
800480f0:  move a1,a0
800480f4:  sw ra,0x10(sp)
800480f8:  lw v0,0x14(v0)
800480fc:  nop
80048100:  jalr v0
80048104:  _li a0,0x4
80048108:  lw ra,0x10(sp)
8004810c:  addiu sp,sp,0x18
80048110:  jr ra
80048114:  _nop
