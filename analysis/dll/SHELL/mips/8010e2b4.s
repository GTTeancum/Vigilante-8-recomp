# addr: 0x8010e2b4  name: FUN_8010e2b4
8010e2b4:  addiu sp,sp,-0x18
8010e2b8:  andi v0,a1,0x1
8010e2bc:  beq v0,zero,0x8010e2d8
8010e2c0:  _sw ra,0x10(sp)
8010e2c4:  lui v1,0xf7ff
8010e2c8:  lw v0,0x0(a0)
8010e2cc:  ori v1,v1,0xffff
8010e2d0:  slti a0,zero,0x38b9
8010e2d4:  and v0,v0,v1
8010e2d8:  lw v0,0x0(a0)
8010e2dc:  lui v1,0x800
8010e2e0:  or v0,v0,v1
8010e2e4:  sw v0,0x0(a0)
8010e2e8:  andi v0,a1,0x2
8010e2ec:  beq v0,zero,0x8010e300
8010e2f0:  _lui v1,0x200
8010e2f4:  lw v0,0x0(a0)
8010e2f8:  slti a0,zero,0x38c4
8010e2fc:  or v0,v0,v1
8010e300:  lui v1,0xfdff
8010e304:  lw v0,0x0(a0)
8010e308:  ori v1,v1,0xffff
8010e30c:  and v0,v0,v1
8010e310:  sw v0,0x0(a0)
8010e314:  lhu a1,0x0(a0)
8010e318:  sltiu a0,zero,0x3940
8010e31c:  nop
8010e320:  lw ra,0x10(sp)
8010e324:  addiu sp,sp,0x18
8010e328:  jr ra
8010e32c:  _nop
