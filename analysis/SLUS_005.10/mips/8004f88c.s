# addr: 0x8004f88c  name: StoreImage
8004f88c:  addiu sp,sp,-0x20
8004f890:  sw s0,0x10(sp)
8004f894:  move s0,a0
8004f898:  sw s1,0x14(sp)
8004f89c:  move s1,a1
8004f8a0:  lui a0,0x8001
8004f8a4:  addiu a0,a0,0x1298
8004f8a8:  sw ra,0x18(sp)
8004f8ac:  jal 0x8004f5e8
8004f8b0:  _move a1,s0
8004f8b4:  move a1,s0
8004f8b8:  lui v0,0x8006
8004f8bc:  lw v0,0x501c(v0)
8004f8c0:  li a2,0x8
8004f8c4:  lw a0,0x1c(v0)
8004f8c8:  lw v0,0x8(v0)
8004f8cc:  nop
8004f8d0:  jalr v0
8004f8d4:  _move a3,s1
8004f8d8:  lw ra,0x18(sp)
8004f8dc:  lw s1,0x14(sp)
8004f8e0:  lw s0,0x10(sp)
8004f8e4:  jr ra
8004f8e8:  _addiu sp,sp,0x20
