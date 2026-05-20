# addr: 0x800191e0  name: FUN_800191e0
800191e0:  lbu v1,0x0(a1)
800191e4:  clear a2
800191e8:  sltiu v0,v1,0x20
800191ec:  bne v0,zero,0x8001922c
800191f0:  _addiu a1,a1,0x1
800191f4:  lw a0,0x0(a0)
800191f8:  nop
800191fc:  lbu a3,0x5(a0)
80019200:  nop
80019204:  subu v1,v1,a3
80019208:  sll v0,v1,0x2
8001920c:  addu v0,v0,v1
80019210:  addu v0,a0,v0
80019214:  lbu v0,0xb(v0)
80019218:  lbu v1,0x0(a1)
8001921c:  addu a2,a2,v0
80019220:  sltiu v0,v1,0x20
80019224:  beq v0,zero,0x80019200
80019228:  _addiu a1,a1,0x1
8001922c:  jr ra
80019230:  _move v0,a2
