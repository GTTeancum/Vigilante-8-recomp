# addr: 0x800156d4  name: FUN_800156d4
800156d4:  lw a0,0x6a4(gp)
800156d8:  lw v0,0x6a8(gp)
800156dc:  nop
800156e0:  bne v0,a0,0x800156f8
800156e4:  _move v1,a0
800156e8:  lw v0,0x6a8(gp)
800156ec:  nop
800156f0:  beq v0,v1,0x800156e8
800156f4:  _nop
800156f8:  lw v0,0x6a8(gp)
800156fc:  nop
80015700:  sw v0,0x6a4(gp)
80015704:  jr ra
80015708:  _move v0,a0
