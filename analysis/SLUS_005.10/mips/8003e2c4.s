# addr: 0x8003e2c4  name: FUN_8003e2c4
8003e2c4:  addiu sp,sp,-0x18
8003e2c8:  sw s0,0x10(sp)
8003e2cc:  move s0,a0
8003e2d0:  beq s0,zero,0x8003e2ec
8003e2d4:  _sw ra,0x14(sp)
8003e2d8:  lw a0,0x0(s0)
8003e2dc:  jal 0x8001bddc
8003e2e0:  _nop
8003e2e4:  jal 0x80045088
8003e2e8:  _move a0,s0
8003e2ec:  lw ra,0x14(sp)
8003e2f0:  lw s0,0x10(sp)
8003e2f4:  jr ra
8003e2f8:  _addiu sp,sp,0x18
