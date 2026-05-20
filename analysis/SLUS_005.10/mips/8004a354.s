# addr: 0x8004a354  name: BIOS_OBJ_DA0
8004a354:  jal 0x800495b4
8004a358:  _nop
8004a35c:  move s0,v0
8004a360:  beq s0,zero,0x8004a3cc
8004a364:  _andi v0,s0,0x4
8004a368:  beq v0,zero,0x8004a398
8004a36c:  _andi v0,s0,0x2
8004a370:  lui v0,0x8006
8004a374:  lw v0,0x80(v0)
8004a378:  nop
8004a37c:  beq v0,zero,0x8004a394
8004a380:  _nop
8004a384:  lbu a0,0x0(s4)
8004a388:  lui a1,0x800a
8004a38c:  jalr v0
8004a390:  _addiu a1,a1,0x3248
8004a394:  andi v0,s0,0x2
8004a398:  beq v0,zero,0x8004a354
8004a39c:  _nop
8004a3a0:  lui v0,0x8006
8004a3a4:  lw v0,0x7c(v0)
8004a3a8:  nop
8004a3ac:  beq v0,zero,0x8004a354
8004a3b0:  _nop
8004a3b4:  lbu a0,0x0(s2)
8004a3b8:  lui a1,0x800a
8004a3bc:  jalr v0
8004a3c0:  _addiu a1,a1,0x3240
8004a3c4:  j 0x8004a354
8004a3c8:  _nop
8004a3cc:  lui v0,0x8006
8004a3d0:  lw v0,0x344(v0)
8004a3d4:  nop
8004a3d8:  sb s1,0x0(v0)
8004a3dc:  lbu v0,0x0(s2)
8004a3e0:  nop
8004a3e4:  beq v0,zero,0x8004a26c
8004a3e8:  _move a2,s6
8004a3ec:  lui a0,0x800a
8004a3f0:  addiu a0,a0,0x3240
8004a3f4:  beq a2,zero,0x8004a418
8004a3f8:  _li v1,0x7
8004a3fc:  li a1,-0x1
8004a400:  lbu v0,0x0(a0)
8004a404:  addiu a0,a0,0x1
8004a408:  addiu v1,v1,-0x1
8004a40c:  sb v0,0x0(a2)
8004a410:  bne v1,a1,0x8004a400
8004a414:  _addiu a2,a2,0x1
8004a418:  clear a0
8004a41c:  lui v0,0x8006
8004a420:  addiu v0,v0,0x35c
8004a424:  lbu v1,0x0(v0)
8004a428:  li v0,0x5
8004a42c:  bne v1,v0,0x8004a43c
8004a430:  _move v0,a0
8004a434:  li a0,-0x1
8004a438:  move v0,a0
