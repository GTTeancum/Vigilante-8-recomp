# addr: 0x8004b870  name: StGetBackloc
8004b870:  lui v0,0x800a
8004b874:  lw v0,0x3270(v0)
8004b878:  addiu sp,sp,-0x18
8004b87c:  sw s0,0x10(sp)
8004b880:  move s0,a0
8004b884:  bne v0,zero,0x8004b8b4
8004b888:  _sw ra,0x14(sp)
8004b88c:  lui a0,0x800a
8004b890:  jal 0x80049534
8004b894:  _addiu a0,a0,0x3278
8004b898:  addiu a0,v0,0x1
8004b89c:  jal 0x80049430
8004b8a0:  _move a1,s0
8004b8a4:  lui v0,0x800a
8004b8a8:  lw v0,0x327c(v0)
8004b8ac:  j 0x8004b8b8
8004b8b0:  _nop
8004b8b4:  li v0,-0x1
