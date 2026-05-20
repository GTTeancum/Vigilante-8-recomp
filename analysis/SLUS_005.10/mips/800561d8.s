# addr: 0x800561d8  name: PADPORTD_OBJ_604
800561d8:  clear a1
800561dc:  li a2,0x10
800561e0:  lui v1,0x800a
800561e4:  addiu v1,v1,0x4d28
800561e8:  beq a0,v1,0x80056208
800561ec:  _move v0,a2
800561f0:  addiu a2,a2,0x10
800561f4:  addiu a1,a1,0x1
800561f8:  slti v0,a1,0x2
800561fc:  bne v0,zero,0x800561e8
80056200:  _addiu v1,v1,0xf0
80056204:  li v0,0xff
80056208:  jr ra
8005620c:  _nop
