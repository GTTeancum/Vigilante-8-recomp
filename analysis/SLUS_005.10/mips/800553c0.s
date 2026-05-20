# addr: 0x800553c0  name: PADCMD_OBJ_72C
800553c0:  addiu v1,v1,0x1
800553c4:  slti v0,v1,0x6
800553c8:  bne v0,zero,0x80055394
800553cc:  _addiu a2,a2,0x1
800553d0:  lbu v0,0xe9(a0)
800553d4:  addiu t0,t0,0x1
800553d8:  slt v0,t0,v0
800553dc:  bne v0,zero,0x80055344
800553e0:  _addiu t2,t2,0x5
800553e4:  li v0,0xfe
800553e8:  sb v0,0x46(a0)
800553ec:  jr ra
800553f0:  _clear v0
