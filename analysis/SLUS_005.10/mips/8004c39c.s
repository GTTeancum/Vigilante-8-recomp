# addr: 0x8004c39c  name: C_011_OBJ_948
8004c39c:  addiu sp,sp,-0x30
8004c3a0:  sw s0,0x18(sp)
8004c3a4:  move s0,a0
8004c3a8:  sw s2,0x20(sp)
8004c3ac:  move s2,a1
8004c3b0:  sw s3,0x24(sp)
8004c3b4:  move s3,a2
8004c3b8:  sw s4,0x28(sp)
8004c3bc:  move s4,a3
8004c3c0:  clear a0
8004c3c4:  sll a1,s0,0x4
8004c3c8:  lui v1,0x100
8004c3cc:  sw ra,0x2c(sp)
8004c3d0:  sw s1,0x1c(sp)
8004c3d4:  lui v0,0x1f80
8004c3d8:  addu v0,v0,a1
8004c3dc:  lw v0,0x1088(v0)
8004c3e0:  lbu s1,0x44(sp)
8004c3e4:  and v0,v0,v1
8004c3e8:  beq v0,zero,0x8004c414
8004c3ec:  _lui a2,0x1
8004c3f0:  beq a0,a2,0x8004c43c
8004c3f4:  _nop
8004c3f8:  lui v0,0x1f80
8004c3fc:  addu v0,v0,a1
8004c400:  lw v0,0x1088(v0)
8004c404:  nop
8004c408:  and v0,v0,v1
8004c40c:  bne v0,zero,0x8004c3f0
8004c410:  _addiu a0,a0,0x1
8004c414:  li v0,0x1
