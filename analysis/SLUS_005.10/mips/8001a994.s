# addr: 0x8001a994  name: FUN_8001a994
8001a994:  addiu sp,sp,-0x20
8001a998:  sw s2,0x18(sp)
8001a99c:  move s2,a0
8001a9a0:  sw ra,0x1c(sp)
8001a9a4:  sw s1,0x14(sp)
8001a9a8:  sw s0,0x10(sp)
8001a9ac:  lw v0,0x0(s2)
8001a9b0:  nop
8001a9b4:  beq v0,zero,0x8001a9f4
8001a9b8:  _nop
8001a9bc:  lw v0,0x10(v0)
8001a9c0:  nop
8001a9c4:  blez v0,0x8001a9f4
8001a9c8:  _clear s0
8001a9cc:  li s1,0xc
8001a9d0:  jal 0x8001884c
8001a9d4:  _addu a0,s2,s1
8001a9d8:  lw v0,0x0(s2)
8001a9dc:  nop
8001a9e0:  lw v0,0x10(v0)
8001a9e4:  addiu s0,s0,0x1
8001a9e8:  slt v0,s0,v0
8001a9ec:  bne v0,zero,0x8001a9d0
8001a9f0:  _addiu s1,s1,0xc
8001a9f4:  lw ra,0x1c(sp)
8001a9f8:  lw s2,0x18(sp)
8001a9fc:  lw s1,0x14(sp)
8001aa00:  lw s0,0x10(sp)
8001aa04:  jr ra
8001aa08:  _addiu sp,sp,0x20
