# addr: 0x800204dc  name: FUN_800204dc
800204dc:  addiu sp,sp,-0x20
800204e0:  sw s1,0x14(sp)
800204e4:  move s1,a0
800204e8:  sw ra,0x18(sp)
800204ec:  beq s1,zero,0x8002052c
800204f0:  _sw s0,0x10(sp)
800204f4:  move a0,s1
800204f8:  jal 0x800203fc
800204fc:  _move s0,s1
80020500:  lw a0,0x30(s1)
80020504:  jal 0x8001bddc
80020508:  _nop
8002050c:  lw a0,0x38(s1)
80020510:  jal 0x800204dc
80020514:  _nop
80020518:  lw s1,0x34(s1)
8002051c:  jal 0x80045088
80020520:  _move a0,s0
80020524:  bne s1,zero,0x800204f4
80020528:  _nop
8002052c:  lw ra,0x18(sp)
80020530:  lw s1,0x14(sp)
80020534:  lw s0,0x10(sp)
80020538:  jr ra
8002053c:  _addiu sp,sp,0x20
