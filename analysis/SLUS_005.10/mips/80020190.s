# addr: 0x80020190  name: FUN_80020190
80020190:  lw v0,0x0(a0)
80020194:  nop
80020198:  lw a3,0x0(v0)
8002019c:  nop
800201a0:  beq a3,zero,0x80020208
800201a4:  _li t0,-0x1
800201a8:  lw a0,0x8(v0)
800201ac:  nop
800201b0:  lh v0,0x6(a0)
800201b4:  nop
800201b8:  slti v0,v0,0x20
800201bc:  bne v0,zero,0x800201f8
800201c0:  _move v0,a3
800201c4:  lw v1,0x0(a0)
800201c8:  nop
800201cc:  and v0,v1,a1
800201d0:  beq v0,zero,0x800201f8
800201d4:  _move v0,a3
800201d8:  andi v0,v1,0x8002
800201dc:  bne v0,zero,0x800201f8
800201e0:  _move v0,a3
800201e4:  addiu a2,a2,-0x1
800201e8:  bne a2,t0,0x800201f8
800201ec:  _nop
800201f0:  jr ra
800201f4:  _move v0,a0
800201f8:  lw a3,0x0(a3)
800201fc:  nop
80020200:  bne a3,zero,0x800201a8
80020204:  _nop
80020208:  jr ra
8002020c:  _clear v0
