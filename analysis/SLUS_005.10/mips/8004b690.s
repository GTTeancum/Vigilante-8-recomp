# addr: 0x8004b690  name: CdGetDiskType
8004b690:  addiu sp,sp,-0x830
8004b694:  li a0,0x1
8004b698:  clear a1
8004b69c:  addiu a2,sp,0x818
8004b6a0:  sw ra,0x828(sp)
8004b6a4:  sw s1,0x824(sp)
8004b6a8:  jal 0x80048fd0
8004b6ac:  _sw s0,0x820(sp)
8004b6b0:  lbu v0,0x818(sp)
8004b6b4:  nop
8004b6b8:  andi v0,v0,0x10
8004b6bc:  bne v0,zero,0x8004b7c4
8004b6c0:  _li v0,0x10
8004b6c4:  li a0,0x10
8004b6c8:  jal 0x80049430
8004b6cc:  _addiu a1,sp,0x10
8004b6d0:  li a0,0x1b
8004b6d4:  addiu a1,sp,0x10
8004b6d8:  jal 0x80048fd0
8004b6dc:  _clear a2
8004b6e0:  clear s0
8004b6e4:  li s1,0x1
8004b6e8:  clear a0
