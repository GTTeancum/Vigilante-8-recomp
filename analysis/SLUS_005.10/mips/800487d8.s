# addr: 0x800487d8  name: INTR_VB_OBJ_C4
800487d8:  lui v0,0x8006
800487dc:  addiu v0,v0,-0x6c
800487e0:  sll a0,a0,0x2
800487e4:  addu a0,a0,v0
800487e8:  lw v0,0x0(a0)
800487ec:  nop
800487f0:  beq a1,v0,0x800487fc
800487f4:  _nop
800487f8:  sw a1,0x0(a0)
800487fc:  jr ra
80048800:  _nop
