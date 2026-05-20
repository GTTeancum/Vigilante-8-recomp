# addr: 0x8001b270  name: FUN_8001b270
8001b270:  addiu sp,sp,-0x18
8001b274:  sw s0,0x10(sp)
8001b278:  sw ra,0x14(sp)
8001b27c:  jal 0x8001b1f8
8001b280:  _move s0,a0
8001b284:  bne v0,zero,0x8001b294
8001b288:  _nop
8001b28c:  j 0x8001b2ec
8001b290:  _clear v0
8001b294:  lw a0,0x58(s0)
8001b298:  lhu v0,0x0(v0)
8001b29c:  lw v1,0x0(a0)
8001b2a0:  nop
8001b2a4:  lw v1,0x4(v1)
8001b2a8:  andi v0,v0,0xfff
8001b2ac:  sll v0,v0,0x2
8001b2b0:  addu v0,v0,v1
8001b2b4:  lw v1,0x0(v0)
8001b2b8:  nop
8001b2bc:  lw v0,0x14(v1)
8001b2c0:  nop
8001b2c4:  lhu v0,0x12(v0)
8001b2c8:  lhu a1,0x12(v1)
8001b2cc:  andi v0,v0,0x3fff
8001b2d0:  addu a1,a1,v0
8001b2d4:  jal 0x8001b3d4
8001b2d8:  _andi a1,a1,0xffff
8001b2dc:  lhu v1,0x8(v0)
8001b2e0:  nop
8001b2e4:  ori v1,v1,0x20
8001b2e8:  sh v1,0x8(v0)
8001b2ec:  lw ra,0x14(sp)
8001b2f0:  lw s0,0x10(sp)
8001b2f4:  jr ra
8001b2f8:  _addiu sp,sp,0x18
