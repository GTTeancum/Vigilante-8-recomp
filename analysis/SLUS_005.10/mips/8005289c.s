# addr: 0x8005289c  name: PRNT_OBJ_258
8005289c:  andi v0,s3,0x1
800528a0:  bne v0,zero,0x800528c4
800528a4:  _andi v0,s3,0x4
800528a8:  beq v0,zero,0x800528c4
800528ac:  _nop
800528b0:  addiu s1,s1,0x4
800528b4:  lw v0,-0x4(s1)
800528b8:  lhu a3,0x38(sp)
800528bc:  j 0x80052ca0
800528c0:  _sh a3,0x0(v0)
800528c4:  addiu s1,s1,0x4
800528c8:  lw v0,-0x4(s1)
800528cc:  lw a3,0x38(sp)
800528d0:  j 0x80052ca0
800528d4:  _sw a3,0x0(v0)
