# addr: 0x800521e8  name: SYS_OBJ_2FE4
800521e8:  beq a2,zero,0x80052204
800521ec:  _addiu v0,a2,-0x1
800521f0:  li v1,-0x1
800521f4:  sb a1,0x0(a0)
800521f8:  addiu v0,v0,-0x1
800521fc:  bne v0,v1,0x800521f4
80052200:  _addiu a0,a0,0x1
80052204:  jr ra
80052208:  _nop
