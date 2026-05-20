# addr: 0x800502a8  name: GetDispEnv
800502a8:  addiu sp,sp,-0x18
800502ac:  sw s0,0x10(sp)
800502b0:  move s0,a0
800502b4:  lui a1,0x8006
800502b8:  addiu a1,a1,0x5090
800502bc:  sw ra,0x14(sp)
800502c0:  jal 0x80044c44
800502c4:  _li a2,0x14
800502c8:  move v0,s0
800502cc:  lw ra,0x14(sp)
800502d0:  lw s0,0x10(sp)
800502d4:  jr ra
800502d8:  _addiu sp,sp,0x18
