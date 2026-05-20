# addr: 0x8003e730  name: FUN_8003e730
8003e730:  addiu sp,sp,-0x18
8003e734:  sw s0,0x10(sp)
8003e738:  move s0,a0
8003e73c:  lui v0,0x8007
8003e740:  lw a0,0x37d4(v0)
8003e744:  sw ra,0x14(sp)
8003e748:  jal 0x8001bda0
8003e74c:  _li a1,0xd
8003e750:  move a0,s0
8003e754:  jal 0x8003e598
8003e758:  _move a1,v0
8003e75c:  lw ra,0x14(sp)
8003e760:  lw s0,0x10(sp)
8003e764:  jr ra
8003e768:  _addiu sp,sp,0x18
