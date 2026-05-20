# addr: 0x8003d188  name: FUN_8003d188
8003d188:  addiu sp,sp,-0x18
8003d18c:  sw s0,0x10(sp)
8003d190:  move s0,a0
8003d194:  move a0,a1
8003d198:  sw ra,0x14(sp)
8003d19c:  lw v0,0x64(a0)
8003d1a0:  nop
8003d1a4:  beq v0,zero,0x8003d1bc
8003d1a8:  _li a1,0xe
8003d1ac:  jalr v0
8003d1b0:  _clear a2
8003d1b4:  j 0x8003d1c0
8003d1b8:  _nop
8003d1bc:  clear v0
8003d1c0:  bne v0,zero,0x8003d1d0
8003d1c4:  _move a0,s0
8003d1c8:  j 0x8003d1d8
8003d1cc:  _clear v0
8003d1d0:  jal 0x8001b038
8003d1d4:  _andi a1,v0,0xffff
8003d1d8:  lw ra,0x14(sp)
8003d1dc:  lw s0,0x10(sp)
8003d1e0:  jr ra
8003d1e4:  _addiu sp,sp,0x18
