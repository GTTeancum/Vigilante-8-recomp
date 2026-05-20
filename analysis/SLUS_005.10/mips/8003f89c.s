# addr: 0x8003f89c  name: FUN_8003f89c
8003f89c:  addiu sp,sp,-0x30
8003f8a0:  sw s4,0x20(sp)
8003f8a4:  move s4,a1
8003f8a8:  sw ra,0x28(sp)
8003f8ac:  sw s5,0x24(sp)
8003f8b0:  sw s3,0x1c(sp)
8003f8b4:  sw s2,0x18(sp)
8003f8b8:  sw s1,0x14(sp)
8003f8bc:  sw s0,0x10(sp)
8003f8c0:  lw a1,0x0(s4)
8003f8c4:  move s1,a0
8003f8c8:  move s0,a3
8003f8cc:  clear s3
8003f8d0:  move s5,a2
8003f8d4:  andi a0,s5,0xffff
8003f8d8:  lw v1,0x0(s1)
8003f8dc:  sll v0,a0,0x3
8003f8e0:  subu v0,v0,a0
8003f8e4:  sll v0,v0,0x2
8003f8e8:  addiu v0,v0,0x1c
8003f8ec:  andi v1,v1,0x4
8003f8f0:  beq v1,zero,0x8003f900
8003f8f4:  _addu s2,a1,v0
8003f8f8:  jal 0x800207f8
8003f8fc:  _move a0,s1
8003f900:  lw a0,0x30(s1)
8003f904:  jal 0x8001bddc
8003f908:  _nop
8003f90c:  lw a0,0x38(s1)
8003f910:  jal 0x800204dc
8003f914:  _nop
8003f918:  lw a0,0x68(s1)
8003f91c:  nop
8003f920:  beq a0,zero,0x8003f938
8003f924:  _nop
8003f928:  jal 0x8001bddc
8003f92c:  _nop
8003f930:  sw zero,0x68(s1)
8003f934:  sw zero,0x6c(s1)
8003f938:  addiu a0,s1,0x10
8003f93c:  addiu a1,s2,0x4
8003f940:  jal 0x80043408
8003f944:  _addiu a2,s1,0x48
8003f948:  lw t0,0x48(s1)
8003f94c:  lw t1,0x4c(s1)
8003f950:  lw t2,0x50(s1)
8003f954:  sw t0,0x24(s1)
8003f958:  sw t1,0x28(s1)
8003f95c:  sw t2,0x2c(s1)
8003f960:  lhu a1,0x0(s2)
8003f964:  ori v0,zero,0xfffd
8003f968:  sltu v0,v0,a1
8003f96c:  bne v0,zero,0x8003f988
8003f970:  _clear v0
8003f974:  move a0,s4
8003f978:  jal 0x8001b49c
8003f97c:  _andi a1,a1,0xff
8003f980:  j 0x8003f98c
8003f984:  _sw v0,0x30(s1)
8003f988:  sw v0,0x30(s1)
8003f98c:  sw zero,0x60(s1)
8003f990:  sh s5,0xa(s1)
8003f994:  beq s0,zero,0x8003f9a0
8003f998:  _sw s0,0x38(s1)
8003f99c:  sw s1,0x3c(s0)
8003f9a0:  lh a1,0x2(s2)
8003f9a4:  nop
8003f9a8:  bltz a1,0x8003f9d0
8003f9ac:  _clear v0
8003f9b0:  lw v0,0x0(s4)
8003f9b4:  nop
8003f9b8:  lw v1,0xc(v0)
8003f9bc:  sll v0,a1,0x2
8003f9c0:  addu v0,v0,v1
8003f9c4:  lw v0,0x0(v0)
8003f9c8:  j 0x8003f9d4
8003f9cc:  _sw v0,0x5c(s1)
8003f9d0:  sw v0,0x5c(s1)
8003f9d4:  jal 0x8001ec48
8003f9d8:  _move a0,s1
8003f9dc:  bne v0,zero,0x8003f9f4
8003f9e0:  _nop
8003f9e4:  lw v0,0x0(s1)
8003f9e8:  nop
8003f9ec:  ori v0,v0,0x20
8003f9f0:  sw v0,0x0(s1)
8003f9f4:  beq s0,zero,0x8003fa28
8003f9f8:  _nop
8003f9fc:  lhu v1,0x6cc(gp)
8003fa00:  lw v0,0x60(s0)
8003fa04:  nop
8003fa08:  beq v0,zero,0x8003fa18
8003fa0c:  _nop
8003fa10:  sh v1,0x46(s0)
8003fa14:  li s3,0x1
8003fa18:  lw s0,0x34(s0)
8003fa1c:  nop
8003fa20:  bne s0,zero,0x8003fa00
8003fa24:  _nop
8003fa28:  beq s3,zero,0x8003fa38
8003fa2c:  _nop
8003fa30:  jal 0x800207c4
8003fa34:  _move a0,s1
8003fa38:  lw v0,0x0(s1)
8003fa3c:  lui v1,0xffff
8003fa40:  ori v1,v1,0x7fff
8003fa44:  move a0,s1
8003fa48:  and v0,v0,v1
8003fa4c:  jal 0x8001dc1c
8003fa50:  _sw v0,0x0(a0)
8003fa54:  lw ra,0x28(sp)
8003fa58:  lw s5,0x24(sp)
8003fa5c:  lw s4,0x20(sp)
8003fa60:  lw s3,0x1c(sp)
8003fa64:  lw s2,0x18(sp)
8003fa68:  lw s1,0x14(sp)
8003fa6c:  lw s0,0x10(sp)
8003fa70:  jr ra
8003fa74:  _addiu sp,sp,0x30
