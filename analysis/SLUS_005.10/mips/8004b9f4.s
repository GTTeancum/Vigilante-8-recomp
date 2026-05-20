# addr: 0x8004b9f4  name: init_ring_status
8004b9f4:  beq a1,zero,0x8004ba20
8004b9f8:  _clear a2
8004b9fc:  addu v0,a2,a0
8004ba00:  addiu a2,a2,0x1
8004ba04:  lui v1,0x800a
8004ba08:  lw v1,0x32c8(v1)
8004ba0c:  sll v0,v0,0x5
8004ba10:  addu v1,v1,v0
8004ba14:  sltu v0,a2,a1
8004ba18:  bne v0,zero,0x8004b9fc
8004ba1c:  _sw zero,0x0(v1)
8004ba20:  jr ra
8004ba24:  _nop
