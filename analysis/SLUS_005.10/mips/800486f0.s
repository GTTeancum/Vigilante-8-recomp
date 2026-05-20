# addr: 0x800486f0  name: INTR_OBJ_69C
800486f0:  beq a1,zero,0x8004870c
800486f4:  _addiu v0,a1,-0x1
800486f8:  li v1,-0x1
800486fc:  sw zero,0x0(a0)
80048700:  addiu v0,v0,-0x1
80048704:  bne v0,v1,0x800486fc
80048708:  _addiu a0,a0,0x4
8004870c:  jr ra
80048710:  _nop
