# addr: 0x80020890  name: FUN_80020890
80020890:  addiu sp,sp,-0x20
80020894:  sw s1,0x14(sp)
80020898:  move s1,a0
8002089c:  sw ra,0x18(sp)
800208a0:  sw s0,0x10(sp)
800208a4:  lw v0,0x0(s1)
800208a8:  nop
800208ac:  andi v0,v0,0x1
800208b0:  beq v0,zero,0x800208c8
800208b4:  _move s0,a1
800208b8:  lui a0,0x8006
800208bc:  addiu a0,a0,0x5ac0
800208c0:  jal 0x8001fe8c
800208c4:  _move a1,s1
800208c8:  lw a0,0x76c(gp)
800208cc:  lw a1,0xc(gp)
800208d0:  lw v1,0x0(a0)
800208d4:  lui v0,0x8006
800208d8:  addiu v0,v0,0x5a70
800208dc:  sw v0,0x4(v1)
800208e0:  sw v1,0x76c(gp)
800208e4:  sw s1,0x8(a0)
800208e8:  lw v0,0x0(s1)
800208ec:  addu s0,s0,a1
800208f0:  ori v0,v0,0x1
800208f4:  sw v0,0x0(s1)
800208f8:  sw s0,0xc(a0)
800208fc:  lw a1,0x7bc(gp)
80020900:  nop
80020904:  lw v1,0x0(a1)
80020908:  nop
8002090c:  beq v1,zero,0x8002093c
80020910:  _nop
80020914:  lw v0,0xc(a1)
80020918:  nop
8002091c:  sltu v0,v0,s0
80020920:  beq v0,zero,0x8002093c
80020924:  _nop
80020928:  move a1,v1
8002092c:  lw v1,0x0(a1)
80020930:  nop
80020934:  bne v1,zero,0x80020914
80020938:  _nop
8002093c:  lw v0,0x4(a1)
80020940:  nop
80020944:  sw a0,0x0(v0)
80020948:  sw a0,0x4(a1)
8002094c:  sw a1,0x0(a0)
80020950:  sw v0,0x4(a0)
80020954:  lw ra,0x18(sp)
80020958:  lw s1,0x14(sp)
8002095c:  lw s0,0x10(sp)
80020960:  jr ra
80020964:  _addiu sp,sp,0x20
