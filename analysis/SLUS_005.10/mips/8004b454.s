# addr: 0x8004b454  name: CDREAD_OBJ_740
8004b454:  bne s2,zero,0x8004b464
8004b458:  _li a0,0x1
8004b45c:  bgtz s0,0x8004b3ec
8004b460:  _nop
8004b464:  jal 0x80048f88
8004b468:  _move a1,s3
8004b46c:  move v0,s0
8004b470:  lw ra,0x20(sp)
8004b474:  lw s3,0x1c(sp)
8004b478:  lw s2,0x18(sp)
8004b47c:  lw s1,0x14(sp)
8004b480:  lw s0,0x10(sp)
8004b484:  jr ra
8004b488:  _addiu sp,sp,0x28
