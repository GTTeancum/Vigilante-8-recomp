# addr: 0x800523a0  name: FUN_800523a0
8005238c:  addiu a2,a2,-0x1
80052390:  bgtz a2,0x800523a0
80052394:  _addiu a1,a1,0x1
80052398:  j 0x800523c8
8005239c:  _clear v0
800523a0:  lbu v1,0x0(a0)
800523a4:  lbu v0,0x0(a1)
800523a8:  nop
800523ac:  beq v1,v0,0x8005238c
800523b0:  _addiu a0,a0,0x1
800523b4:  addiu a0,a0,-0x1
800523b8:  lbu v1,0x0(a0)
800523bc:  lbu v0,0x0(a1)
800523c0:  nop
800523c4:  subu v0,v1,v0
800523c8:  jr ra
800523cc:  _nop
