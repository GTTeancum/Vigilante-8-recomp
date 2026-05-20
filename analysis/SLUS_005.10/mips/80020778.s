# addr: 0x80020778  name: FUN_80020778
80020778:  addiu sp,sp,-0x18
8002077c:  move a1,a0
80020780:  sw ra,0x10(sp)
80020784:  lw v1,0x0(a1)
80020788:  nop
8002078c:  andi v0,v1,0x80
80020790:  bne v0,zero,0x800207a0
80020794:  _li v0,-0x81
80020798:  j 0x800207b4
8002079c:  _clear v0
800207a0:  and v0,v1,v0
800207a4:  lui a0,0x8006
800207a8:  addiu a0,a0,0x5a60
800207ac:  jal 0x8001fe8c
800207b0:  _sw v0,0x0(a1)
800207b4:  lw ra,0x10(sp)
800207b8:  nop
800207bc:  jr ra
800207c0:  _addiu sp,sp,0x18
