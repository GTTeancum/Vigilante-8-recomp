# addr: 0x8005633c  name: PADSEQD_OBJ_108
8005633c:  addiu sp,sp,-0x20
80056340:  sw s0,0x10(sp)
80056344:  move s0,a0
80056348:  sw ra,0x18(sp)
8005634c:  sw s1,0x14(sp)
80056350:  lw v0,0x3c(s0)
80056354:  li v1,0xf
80056358:  lbu v0,0x0(v0)
8005635c:  lbu s1,0xe8(s0)
80056360:  srl v0,v0,0x4
80056364:  bne v0,v1,0x80056374
80056368:  _sb v0,0xe8(s0)
8005636c:  j 0x800563d4
80056370:  _sb s1,0xe8(s0)
80056374:  lw v0,0x30(s0)
80056378:  nop
8005637c:  sb zero,0x0(v0)
80056380:  lw v0,0x3c(s0)
80056384:  lw v1,0x30(s0)
80056388:  lbu v0,0x0(v0)
8005638c:  nop
80056390:  sb v0,0x1(v1)
80056394:  lbu v0,0x44(s0)
80056398:  li a0,0x2
8005639c:  slt v0,a0,v0
800563a0:  beq v0,zero,0x800563d4
800563a4:  _nop
800563a8:  lw v0,0x3c(s0)
800563ac:  lw v1,0x30(s0)
800563b0:  addu v0,v0,a0
800563b4:  lbu v0,0x0(v0)
800563b8:  addu v1,v1,a0
800563bc:  sb v0,0x0(v1)
800563c0:  lbu v0,0x44(s0)
800563c4:  addiu a0,a0,0x1
800563c8:  slt v0,a0,v0
800563cc:  bne v0,zero,0x800563a8
800563d0:  _nop
