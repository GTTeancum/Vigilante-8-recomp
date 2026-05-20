# addr: 0x8004c474  name: C_011_OBJ_A20
8004c474:  sb v0,0x2(v1)
8004c478:  lui v0,0x8006
8004c47c:  lw v0,0x450(v0)
8004c480:  nop
8004c484:  lw v0,0x0(v0)
8004c488:  nop
8004c48c:  sw v0,0x10(sp)
8004c490:  sll a2,s0,0x2
8004c494:  addiu a2,a2,0x3
8004c498:  li v1,0x1
8004c49c:  sllv v1,v1,a2
8004c4a0:  lui a1,0x1f80
8004c4a4:  ori a1,a1,0x1080
8004c4a8:  sll v0,s0,0x4
8004c4ac:  addu a1,v0,a1
8004c4b0:  lui a0,0x8006
8004c4b4:  lw a0,0x44c(a0)
8004c4b8:  sll v0,s3,0x10
8004c4bc:  lw a2,0x0(a0)
8004c4c0:  or v0,v0,s4
8004c4c4:  or a2,a2,v1
8004c4c8:  sw a2,0x0(a0)
8004c4cc:  sw s2,0x0(a1)
8004c4d0:  addiu a1,a1,0x4
8004c4d4:  sw v0,0x0(a1)
8004c4d8:  lui v1,0x8006
8004c4dc:  lw v1,0x434(v1)
8004c4e0:  nop
8004c4e4:  lbu v0,0x0(v1)
8004c4e8:  nop
8004c4ec:  andi v0,v0,0x40
8004c4f0:  bne v0,zero,0x8004c50c
8004c4f4:  _addiu a1,a1,0x4
8004c4f8:  lbu v0,0x0(v1)
8004c4fc:  nop
8004c500:  andi v0,v0,0x40
8004c504:  beq v0,zero,0x8004c4f8
8004c508:  _nop
8004c50c:  lw v0,0x40(sp)
8004c510:  nop
8004c514:  sw v0,0x0(a1)
8004c518:  lw v0,0x0(a1)
8004c51c:  nop
8004c520:  sw v0,0x10(sp)
8004c524:  lw ra,0x2c(sp)
8004c528:  lw s4,0x28(sp)
8004c52c:  lw s3,0x24(sp)
8004c530:  lw s2,0x20(sp)
8004c534:  lw s1,0x1c(sp)
8004c538:  lw s0,0x18(sp)
8004c53c:  jr ra
8004c540:  _addiu sp,sp,0x30
