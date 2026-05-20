# addr: 0x80021978  name: FUN_80021978
80021978:  lw v0,0x0(a0)
8002197c:  nop
80021980:  lw t0,0x0(v0)
80021984:  nop
80021988:  beq t0,zero,0x80021a28
8002198c:  _nop
80021990:  lw a0,0x8(v0)
80021994:  nop
80021998:  lh v1,0x6(a0)
8002199c:  nop
800219a0:  slt v0,v1,a1
800219a4:  bne v0,zero,0x80021a18
800219a8:  _move v0,t0
800219ac:  slt v0,a2,v1
800219b0:  bne v0,zero,0x80021a18
800219b4:  _move v0,t0
800219b8:  lw v1,0x48(a0)
800219bc:  lw v0,0x0(a3)
800219c0:  nop
800219c4:  slt v0,v0,v1
800219c8:  beq v0,zero,0x80021a18
800219cc:  _move v0,t0
800219d0:  lw v0,0x4(a3)
800219d4:  nop
800219d8:  slt v0,v1,v0
800219dc:  beq v0,zero,0x80021a18
800219e0:  _move v0,t0
800219e4:  lw v1,0x50(a0)
800219e8:  lw v0,0x8(a3)
800219ec:  nop
800219f0:  slt v0,v0,v1
800219f4:  beq v0,zero,0x80021a18
800219f8:  _move v0,t0
800219fc:  lw v0,0xc(a3)
80021a00:  nop
80021a04:  slt v0,v1,v0
80021a08:  beq v0,zero,0x80021a18
80021a0c:  _move v0,t0
80021a10:  jr ra
80021a14:  _move v0,a0
80021a18:  lw t0,0x0(t0)
80021a1c:  nop
80021a20:  bne t0,zero,0x80021990
80021a24:  _nop
80021a28:  jr ra
80021a2c:  _clear v0
