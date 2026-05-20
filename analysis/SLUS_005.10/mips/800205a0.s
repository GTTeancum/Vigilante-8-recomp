# addr: 0x800205a0  name: FUN_800205a0
800205a0:  addiu sp,sp,-0x18
800205a4:  move a1,a0
800205a8:  beq a1,zero,0x800205e8
800205ac:  _sw ra,0x10(sp)
800205b0:  lw v1,0x4(a1)
800205b4:  lw v0,0x0(a1)
800205b8:  lw a0,0x8(a1)
800205bc:  sw v1,0x4(v0)
800205c0:  sw v0,0x0(v1)
800205c4:  lw v0,0x774(gp)
800205c8:  sw a1,0x774(gp)
800205cc:  sw a1,0x0(v0)
800205d0:  sw v0,0x4(a1)
800205d4:  lui v0,0x8006
800205d8:  addiu v0,v0,0x5a74
800205dc:  sw v0,0x0(a1)
800205e0:  jal 0x80020540
800205e4:  _sw zero,0x8(a1)
800205e8:  lw ra,0x10(sp)
800205ec:  nop
800205f0:  jr ra
800205f4:  _addiu sp,sp,0x18
