# addr: 0x800187e4  name: FUN_800187e4
800187e4:  addiu sp,sp,-0x18
800187e8:  sw s0,0x10(sp)
800187ec:  move s0,a1
800187f0:  addiu a1,s0,0x8
800187f4:  addiu a2,s0,0xa
800187f8:  sw ra,0x14(sp)
800187fc:  jal 0x80018618
80018800:  _addiu a3,s0,0x6
80018804:  lw v1,0xc(v0)
80018808:  lw a0,0x0(v0)
8001880c:  lh a1,0x4(v1)
80018810:  andi a0,a0,0x3
80018814:  li v1,0x2
80018818:  subu v1,v1,a0
8001881c:  sllv a1,a1,v1
80018820:  sh a1,0x2(s0)
80018824:  lw v1,0xc(v0)
80018828:  nop
8001882c:  lh a0,0x6(v1)
80018830:  li v1,0x1
80018834:  sh v1,0x0(s0)
80018838:  sh a0,0x4(s0)
8001883c:  lw ra,0x14(sp)
80018840:  lw s0,0x10(sp)
80018844:  jr ra
80018848:  _addiu sp,sp,0x18
