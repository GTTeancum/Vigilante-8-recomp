# addr: 0x80023394  name: FUN_80023394
80023394:  addiu sp,sp,-0x30
80023398:  sw ra,0x28(sp)
8002339c:  sw s5,0x24(sp)
800233a0:  sw s4,0x20(sp)
800233a4:  sw s3,0x1c(sp)
800233a8:  sw s2,0x18(sp)
800233ac:  sw s1,0x14(sp)
800233b0:  sw s0,0x10(sp)
800233b4:  lw a0,0x0(a0)
800233b8:  nop
800233bc:  lw s1,0x0(a0)
800233c0:  move s4,a1
800233c4:  move s5,a2
800233c8:  li s2,-0x1
800233cc:  beq s1,zero,0x80023444
800233d0:  _clear s3
800233d4:  lw s0,0x8(a0)
800233d8:  nop
800233dc:  lh v0,0x6(s0)
800233e0:  nop
800233e4:  slti v0,v0,0x20
800233e8:  bne v0,zero,0x80023434
800233ec:  _move a0,s1
800233f0:  lw v1,0x0(s0)
800233f4:  nop
800233f8:  andi v0,v1,0x4000
800233fc:  beq v0,zero,0x80023434
80023400:  _nop
80023404:  and v0,v1,s4
80023408:  beq v0,zero,0x80023434
8002340c:  _nop
80023410:  move a0,s5
80023414:  jal 0x80016aac
80023418:  _addiu a1,s0,0x48
8002341c:  move v1,v0
80023420:  sltu v0,v1,s2
80023424:  beq v0,zero,0x80023434
80023428:  _move a0,s1
8002342c:  move s2,v1
80023430:  move s3,s0
80023434:  lw s1,0x0(s1)
80023438:  nop
8002343c:  bne s1,zero,0x800233d4
80023440:  _nop
80023444:  lw ra,0x28(sp)
80023448:  lw s5,0x24(sp)
8002344c:  lw s4,0x20(sp)
80023450:  move v0,s3
80023454:  lw s3,0x1c(sp)
80023458:  lw s2,0x18(sp)
8002345c:  lw s1,0x14(sp)
80023460:  lw s0,0x10(sp)
80023464:  jr ra
80023468:  _addiu sp,sp,0x30
