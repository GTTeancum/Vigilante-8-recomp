# addr: 0x800166dc  name: FUN_800166dc
800166dc:  lw v0,0x0(a0)
800166e0:  nop
800166e4:  lw v0,0x0(v0)
800166e8:  nop
800166ec:  beq v0,zero,0x80016704
800166f0:  _clear v1
800166f4:  lw v0,0x0(v0)
800166f8:  nop
800166fc:  bne v0,zero,0x800166f4
80016700:  _addiu v1,v1,0x1
80016704:  jr ra
80016708:  _move v0,v1
