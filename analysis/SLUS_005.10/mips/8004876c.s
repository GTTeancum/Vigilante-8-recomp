# addr: 0x8004876c  name: INTR_VB_OBJ_58
8004876c:  lui v0,0x8006
80048770:  lw v0,-0x4c(v0)
80048774:  addiu sp,sp,-0x20
80048778:  sw s1,0x14(sp)
8004877c:  clear s1
80048780:  sw s0,0x10(sp)
80048784:  lui s0,0x8006
80048788:  addiu s0,s0,-0x6c
8004878c:  sw ra,0x18(sp)
80048790:  addiu v0,v0,0x1
80048794:  lui at,0x8006
80048798:  sw v0,-0x4c(at)
8004879c:  lw v0,0x0(s0)
800487a0:  nop
800487a4:  beq v0,zero,0x800487b4
800487a8:  _nop
800487ac:  jalr v0
800487b0:  _nop
800487b4:  addiu s1,s1,0x1
800487b8:  slti v0,s1,0x8
800487bc:  bne v0,zero,0x8004879c
800487c0:  _addiu s0,s0,0x4
800487c4:  lw ra,0x18(sp)
800487c8:  lw s1,0x14(sp)
800487cc:  lw s0,0x10(sp)
800487d0:  jr ra
800487d4:  _addiu sp,sp,0x20
