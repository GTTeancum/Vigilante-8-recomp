# addr: 0x8005607c  name: PADPORTD_OBJ_4A8
8005607c:  beq a2,zero,0x800560b8
80056080:  _nop
80056084:  addiu a1,s0,0x5d
80056088:  addiu a0,s0,0x57
8005608c:  beq t1,zero,0x800560b8
80056090:  _clear v1
80056094:  lbu v0,0x0(a1)
80056098:  nop
8005609c:  bne v0,t0,0x800560a8
800560a0:  _addiu a1,a1,0x1
800560a4:  sb t3,0x0(a0)
800560a8:  addiu v1,v1,0x1
800560ac:  slt v0,v1,t1
800560b0:  bne v0,zero,0x80056094
800560b4:  _addiu a0,a0,0x1
800560b8:  lbu v0,0xe9(s0)
800560bc:  addiu t0,t0,0x1
800560c0:  slt v0,t0,v0
800560c4:  bne v0,zero,0x80055fc8
800560c8:  _addiu t2,t2,0x5
800560cc:  j 0x800561c8
800560d0:  _nop
