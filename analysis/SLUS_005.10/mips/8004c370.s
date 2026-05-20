# addr: 0x8004c370  name: C_011_OBJ_91C
8004c370:  beq a2,zero,0x8004c394
8004c374:  _clear v1
8004c378:  lw v0,0x0(a1)
8004c37c:  addiu a1,a1,0x4
8004c380:  addiu v1,v1,0x1
8004c384:  sw v0,0x0(a0)
8004c388:  sltu v0,v1,a2
8004c38c:  bne v0,zero,0x8004c378
8004c390:  _addiu a0,a0,0x4
8004c394:  jr ra
8004c398:  _nop
