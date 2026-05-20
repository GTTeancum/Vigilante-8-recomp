# addr: 0x80020620  name: FUN_80020620
80020620:  lw v0,0x730(gp)
80020624:  addiu sp,sp,-0x18
80020628:  sw s0,0x10(sp)
8002062c:  move s0,a0
80020630:  move a2,a1
80020634:  sw ra,0x14(sp)
80020638:  jalr v0
8002063c:  _li a1,0x11
80020640:  jal 0x800205f8
80020644:  _move a0,s0
80020648:  lw ra,0x14(sp)
8002064c:  lw s0,0x10(sp)
80020650:  jr ra
80020654:  _addiu sp,sp,0x18
