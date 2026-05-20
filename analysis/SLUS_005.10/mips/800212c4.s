# addr: 0x800212c4  name: FUN_800212c4
800212c4:  lw v0,0x77c(gp)
800212c8:  addiu sp,sp,-0x20
800212cc:  sw ra,0x18(sp)
800212d0:  sw s1,0x14(sp)
800212d4:  sw s0,0x10(sp)
800212d8:  lw s0,0x0(v0)
800212dc:  nop
800212e0:  beq s0,zero,0x80021308
800212e4:  _move s1,a0
800212e8:  lw a0,0x8(v0)
800212ec:  jal 0x8001fcb4
800212f0:  _andi a1,s1,0xffff
800212f4:  move v0,s0
800212f8:  lw s0,0x0(s0)
800212fc:  nop
80021300:  bne s0,zero,0x800212e8
80021304:  _nop
80021308:  lw ra,0x18(sp)
8002130c:  lw s1,0x14(sp)
80021310:  lw s0,0x10(sp)
80021314:  jr ra
80021318:  _addiu sp,sp,0x20
