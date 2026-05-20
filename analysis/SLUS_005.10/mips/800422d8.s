# addr: 0x800422d8  name: FUN_800422d8
800422d8:  addiu sp,sp,-0x28
800422dc:  sw s1,0x14(sp)
800422e0:  move s1,a0
800422e4:  lw a0,0x8c4(gp)
800422e8:  sw ra,0x20(sp)
800422ec:  sw s3,0x1c(sp)
800422f0:  sw s2,0x18(sp)
800422f4:  sw s0,0x10(sp)
800422f8:  lw s0,0x0(a0)
800422fc:  move s2,a1
80042300:  beq s0,zero,0x80042374
80042304:  _move s3,a2
80042308:  lw v0,0x14(a0)
8004230c:  lw v1,0xc(a0)
80042310:  subu v0,v0,s2
80042314:  bgez v0,0x80042320
80042318:  _move a1,v0
8004231c:  subu a1,zero,a1
80042320:  subu v1,v1,s1
80042324:  bgez v1,0x80042330
80042328:  _nop
8004232c:  subu v1,zero,v1
80042330:  slt v0,a1,v1
80042334:  beq v0,zero,0x80042340
80042338:  _nop
8004233c:  move a1,v1
80042340:  lw v0,0x18(a0)
80042344:  nop
80042348:  addu v0,v0,s3
8004234c:  slt v0,a1,v0
80042350:  beq v0,zero,0x80042360
80042354:  _nop
80042358:  jal 0x800420f4
8004235c:  _nop
80042360:  move a0,s0
80042364:  lw s0,0x0(s0)
80042368:  nop
8004236c:  bne s0,zero,0x80042308
80042370:  _nop
80042374:  lw ra,0x20(sp)
80042378:  lw s3,0x1c(sp)
8004237c:  lw s2,0x18(sp)
80042380:  lw s1,0x14(sp)
80042384:  lw s0,0x10(sp)
80042388:  jr ra
8004238c:  _addiu sp,sp,0x28
