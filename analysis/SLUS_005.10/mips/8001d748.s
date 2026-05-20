# addr: 0x8001d748  name: FUN_8001d748
8001d748:  addiu sp,sp,-0x28
8001d74c:  sw s4,0x20(sp)
8001d750:  move s4,a0
8001d754:  sw s0,0x10(sp)
8001d758:  move s0,a1
8001d75c:  sw ra,0x24(sp)
8001d760:  sw s3,0x1c(sp)
8001d764:  sw s2,0x18(sp)
8001d768:  sw s1,0x14(sp)
8001d76c:  lw a0,0x0(s0)
8001d770:  lw a1,0x8(s0)
8001d774:  move s2,a2
8001d778:  jal 0x80025400
8001d77c:  _move s3,a3
8001d780:  lw a0,0x74(s4)
8001d784:  nop
8001d788:  beq a0,zero,0x8001d7dc
8001d78c:  _move s1,v0
8001d790:  move a1,s1
8001d794:  move a2,s0
8001d798:  jal 0x8001f51c
8001d79c:  _move a3,s2
8001d7a0:  bne v0,zero,0x8001d7cc
8001d7a4:  _nop
8001d7a8:  lw a0,0x78(s4)
8001d7ac:  nop
8001d7b0:  beq a0,zero,0x8001d7dc
8001d7b4:  _move a1,s1
8001d7b8:  move a2,s0
8001d7bc:  jal 0x8001f51c
8001d7c0:  _move a3,s2
8001d7c4:  beq v0,zero,0x8001d7dc
8001d7c8:  _nop
8001d7cc:  beq s3,zero,0x8001d820
8001d7d0:  _nop
8001d7d4:  j 0x8001d820
8001d7d8:  _sw zero,0x0(s3)
8001d7dc:  beq s2,zero,0x8001d800
8001d7e0:  _nop
8001d7e4:  lw a0,0x0(s0)
8001d7e8:  lw a1,0x8(s0)
8001d7ec:  jal 0x80025648
8001d7f0:  _move a2,s2
8001d7f4:  move a0,s2
8001d7f8:  jal 0x8004c844
8001d7fc:  _move a1,a0
8001d800:  beq s3,zero,0x8001d820
8001d804:  _move v0,s1
8001d808:  lw a0,0x0(s0)
8001d80c:  lw a1,0x8(s0)
8001d810:  jal 0x800255f4
8001d814:  _nop
8001d818:  sw v0,0x0(s3)
8001d81c:  move v0,s1
8001d820:  lw ra,0x24(sp)
8001d824:  lw s4,0x20(sp)
8001d828:  lw s3,0x1c(sp)
8001d82c:  lw s2,0x18(sp)
8001d830:  lw s1,0x14(sp)
8001d834:  lw s0,0x10(sp)
8001d838:  jr ra
8001d83c:  _addiu sp,sp,0x28
