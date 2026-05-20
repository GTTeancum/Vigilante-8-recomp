# addr: 0x80021394  name: FUN_80021394
80021394:  lw v0,0x7c4(gp)
80021398:  addiu sp,sp,-0x20
8002139c:  sw s1,0x14(sp)
800213a0:  move s1,a0
800213a4:  lui v1,0x8006
800213a8:  addiu v1,v1,0x5ac0
800213ac:  sw ra,0x18(sp)
800213b0:  beq v0,v1,0x8002144c
800213b4:  _sw s0,0x10(sp)
800213b8:  move s0,v1
800213bc:  lw a1,0x7bc(gp)
800213c0:  nop
800213c4:  lw v0,0xc(a1)
800213c8:  nop
800213cc:  sltu v0,s1,v0
800213d0:  bne v0,zero,0x8002144c
800213d4:  _li v1,-0x2
800213d8:  lw a0,0x8(a1)
800213dc:  nop
800213e0:  lw v0,0x0(a0)
800213e4:  nop
800213e8:  and v0,v0,v1
800213ec:  sw v0,0x0(a0)
800213f0:  lw v1,0x4(a1)
800213f4:  lw v0,0x0(a1)
800213f8:  nop
800213fc:  sw v1,0x4(v0)
80021400:  sw v0,0x0(v1)
80021404:  lw v0,0x774(gp)
80021408:  sw a1,0x774(gp)
8002140c:  sw a1,0x0(v0)
80021410:  sw v0,0x4(a1)
80021414:  lui v0,0x8006
80021418:  addiu v0,v0,0x5a74
8002141c:  sw v0,0x0(a1)
80021420:  sw zero,0x8(a1)
80021424:  lw v0,0x64(a0)
80021428:  nop
8002142c:  beq v0,zero,0x8002143c
80021430:  _li a1,0x2
80021434:  jalr v0
80021438:  _clear a2
8002143c:  lw v0,0x7c4(gp)
80021440:  nop
80021444:  bne v0,s0,0x800213bc
80021448:  _nop
8002144c:  lw ra,0x18(sp)
80021450:  lw s1,0x14(sp)
80021454:  lw s0,0x10(sp)
80021458:  jr ra
8002145c:  _addiu sp,sp,0x20
