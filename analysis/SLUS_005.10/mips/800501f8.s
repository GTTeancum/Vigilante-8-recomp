# addr: 0x800501f8  name: SYS_OBJ_FF4
800501f8:  move s0,a1
800501fc:  addiu a1,s0,0x2
80050200:  slt v0,s2,a1
80050204:  bne v0,zero,0x8005021c
80050208:  _slti v0,s2,0x103
8005020c:  li a1,0x102
