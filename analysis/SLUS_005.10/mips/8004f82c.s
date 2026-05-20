# addr: 0x8004f82c  name: LoadImage
8004f82c:  addiu sp,sp,-0x20
8004f830:  sw s0,0x10(sp)
8004f834:  move s0,a0
8004f838:  sw s1,0x14(sp)
8004f83c:  move s1,a1
8004f840:  lui a0,0x8001
8004f844:  addiu a0,a0,0x128c
8004f848:  sw ra,0x18(sp)
8004f84c:  jal 0x8004f5e8
8004f850:  _move a1,s0
8004f854:  move a1,s0
8004f858:  lui v0,0x8006
8004f85c:  lw v0,0x501c(v0)
8004f860:  li a2,0x8
8004f864:  lw a0,0x20(v0)
8004f868:  lw v0,0x8(v0)
8004f86c:  nop
8004f870:  jalr v0
8004f874:  _move a3,s1
8004f878:  lw ra,0x18(sp)
8004f87c:  lw s1,0x14(sp)
8004f880:  lw s0,0x10(sp)
8004f884:  jr ra
8004f888:  _addiu sp,sp,0x20
