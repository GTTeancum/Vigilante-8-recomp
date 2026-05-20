# addr: 0x8004f8ec  name: MoveImage
8004f8ec:  addiu sp,sp,-0x20
8004f8f0:  sw s0,0x10(sp)
8004f8f4:  move s0,a0
8004f8f8:  sw s2,0x18(sp)
8004f8fc:  move s2,a1
8004f900:  sw s1,0x14(sp)
8004f904:  move s1,a2
8004f908:  lui a0,0x8001
8004f90c:  addiu a0,a0,0x12a4
8004f910:  sw ra,0x1c(sp)
8004f914:  jal 0x8004f5e8
8004f918:  _move a1,s0
8004f91c:  lh v0,0x4(s0)
8004f920:  nop
8004f924:  beq v0,zero,0x8004f98c
8004f928:  _li v0,-0x1
8004f92c:  lh v0,0x6(s0)
8004f930:  nop
8004f934:  bne v0,zero,0x8004f944
8004f938:  _sll v0,s1,0x10
8004f93c:  j 0x8004f98c
8004f940:  _li v0,-0x1
8004f944:  lui v1,0x8006
8004f948:  addiu v1,v1,0x50c4
8004f94c:  andi a0,s2,0xffff
8004f950:  or v0,v0,a0
8004f954:  lw a1,0x0(s0)
8004f958:  lui a3,0x8006
8004f95c:  lw a3,0x501c(a3)
8004f960:  li a2,0x14
8004f964:  sw v0,0x4(v1)
8004f968:  sw a1,0x0(v1)
8004f96c:  lw v0,0x4(s0)
8004f970:  addiu a1,v1,-0x8
8004f974:  sw v0,0x8(v1)
8004f978:  lw a0,0x18(a3)
8004f97c:  lw v0,0x8(a3)
8004f980:  nop
8004f984:  jalr v0
8004f988:  _clear a3
