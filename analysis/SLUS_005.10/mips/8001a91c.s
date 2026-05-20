# addr: 0x8001a91c  name: FUN_8001a91c
8001a91c:  addiu sp,sp,-0x20
8001a920:  sw s2,0x18(sp)
8001a924:  move s2,a0
8001a928:  sw ra,0x1c(sp)
8001a92c:  sw s1,0x14(sp)
8001a930:  sw s0,0x10(sp)
8001a934:  lw s1,0x0(s2)
8001a938:  nop
8001a93c:  lw v0,0x10(s1)
8001a940:  nop
8001a944:  blez v0,0x8001a96c
8001a948:  _clear s0
8001a94c:  move a0,s2
8001a950:  jal 0x8001b3d4
8001a954:  _andi a1,s0,0xffff
8001a958:  lw v0,0x10(s1)
8001a95c:  addiu s0,s0,0x1
8001a960:  slt v0,s0,v0
8001a964:  bne v0,zero,0x8001a950
8001a968:  _move a0,s2
8001a96c:  lw a1,0x14(s1)
8001a970:  move a0,s1
8001a974:  jal 0x80045134
8001a978:  _subu a1,a1,a0
8001a97c:  lw ra,0x1c(sp)
8001a980:  lw s2,0x18(sp)
8001a984:  lw s1,0x14(sp)
8001a988:  lw s0,0x10(sp)
8001a98c:  jr ra
8001a990:  _addiu sp,sp,0x20
