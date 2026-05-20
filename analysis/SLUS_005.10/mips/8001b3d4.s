# addr: 0x8001b3d4  name: FUN_8001b3d4
8001b3d4:  addiu sp,sp,-0x18
8001b3d8:  andi a1,a1,0xffff
8001b3dc:  sll v0,a1,0x1
8001b3e0:  addu v0,v0,a1
8001b3e4:  sll v0,v0,0x2
8001b3e8:  addiu v0,v0,0xc
8001b3ec:  sw s0,0x10(sp)
8001b3f0:  addu s0,a0,v0
8001b3f4:  sw ra,0x14(sp)
8001b3f8:  lhu v0,0x0(s0)
8001b3fc:  li v1,0x1
8001b400:  addiu v0,v0,0x1
8001b404:  sh v0,0x0(s0)
8001b408:  andi v0,v0,0xffff
8001b40c:  bne v0,v1,0x8001b434
8001b410:  _nop
8001b414:  lw v0,0x0(a0)
8001b418:  nop
8001b41c:  lw v1,0x14(v0)
8001b420:  sll v0,a1,0x2
8001b424:  addu v0,v0,v1
8001b428:  lw a0,0x0(v0)
8001b42c:  jal 0x800187e4
8001b430:  _move a1,s0
8001b434:  lw ra,0x14(sp)
8001b438:  move v0,s0
8001b43c:  lw s0,0x10(sp)
8001b440:  jr ra
8001b444:  _addiu sp,sp,0x18
