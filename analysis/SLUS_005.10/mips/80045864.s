# addr: 0x80045864  name: SPU_OBJ_390
80045864:  jal 0x80045fac
80045868:  _subu s1,s1,s0
8004586c:  jal 0x80045fac
80045870:  _nop
80045874:  bne s1,zero,0x8004579c
80045878:  _sltiu v0,s1,0x41
8004587c:  lui v0,0x8006
80045880:  lw v0,-0x122c(v0)
80045884:  nop
80045888:  lhu a0,0x1aa(v0)
8004588c:  andi a1,s3,0xffff
80045890:  andi v1,a0,0xffcf
80045894:  sh v1,0x1aa(v0)
80045898:  lhu v0,0x1ae(v0)
8004589c:  nop
800458a0:  andi v0,v0,0x7ff
800458a4:  beq v0,a1,0x800458f8
800458a8:  _clear v1
800458ac:  addiu v1,v1,0x1
800458b0:  sltiu v0,v1,0xf01
800458b4:  bne v0,zero,0x800458d8
800458b8:  _nop
800458bc:  lui a0,0x8001
800458c0:  addiu a0,a0,0xb94
800458c4:  lui a1,0x8001
800458c8:  jal 0x80052604
800458cc:  _addiu a1,a1,0xbc8
800458d0:  j 0x800458f8
800458d4:  _nop
800458d8:  lui v0,0x8006
800458dc:  lw v0,-0x122c(v0)
800458e0:  nop
800458e4:  lhu v0,0x1ae(v0)
800458e8:  nop
800458ec:  andi v0,v0,0x7ff
800458f0:  bne v0,a1,0x800458b0
800458f4:  _addiu v1,v1,0x1
