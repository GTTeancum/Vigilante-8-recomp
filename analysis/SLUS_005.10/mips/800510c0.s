# addr: 0x800510c0  name: SYS_OBJ_1EBC
800510c0:  lh a1,0x6(s1)
800510c4:  lhu v1,0x6(s1)
800510c8:  bltz a1,0x800510f8
800510cc:  _sh a0,0x4(s1)
800510d0:  move a0,v1
800510d4:  lui v0,0x8006
800510d8:  lh v0,0x502a(v0)
800510dc:  lui v1,0x8006
800510e0:  lhu v1,0x502a(v1)
800510e4:  slt v0,v0,a1
800510e8:  beq v0,zero,0x80051100
800510ec:  _sll v0,a0,0x10
800510f0:  j 0x800510fc
800510f4:  _move a0,v1
800510f8:  clear a0
