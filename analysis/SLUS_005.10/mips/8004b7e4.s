# addr: 0x8004b7e4  name: data_ready_callback
8004b7e4:  lui v0,0x800a
8004b7e8:  lw v0,0x32b0(v0)
8004b7ec:  lui v1,0x800a
8004b7f0:  lw v1,0x32c8(v1)
8004b7f4:  addiu sp,sp,-0x18
8004b7f8:  sw ra,0x10(sp)
8004b7fc:  sll v0,v0,0x5
8004b800:  addu v1,v1,v0
8004b804:  li v0,0x2
8004b808:  sh v0,0x0(v1)
8004b80c:  lui a2,0x800a
8004b810:  addiu a2,a2,0x3278
8004b814:  lwl v0,0x1f(v1)
8004b818:  lwr v0,0x1c(v1)
8004b81c:  nop
8004b820:  swl v0,0x3(a2)
8004b824:  swr v0,0x0(a2)
8004b828:  lw v0,0x8(v1)
8004b82c:  lui v1,0x800a
8004b830:  lw v1,0x32ac(v1)
8004b834:  lui a0,0x800a
8004b838:  lw a0,0x32d0(a0)
8004b83c:  lui at,0x800a
8004b840:  sw v0,0x327c(at)
8004b844:  lui at,0x800a
8004b848:  beq a0,zero,0x8004b858
8004b84c:  _sw v1,0x32b0(at)
8004b850:  jalr a0
8004b854:  _nop
8004b858:  lui at,0x800a
8004b85c:  sw zero,0x32a4(at)
8004b860:  lw ra,0x10(sp)
8004b864:  addiu sp,sp,0x18
8004b868:  jr ra
8004b86c:  _nop
