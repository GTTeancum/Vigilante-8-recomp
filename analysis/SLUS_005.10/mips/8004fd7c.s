# addr: 0x8004fd7c  name: GetDrawEnv
8004fd7c:  addiu sp,sp,-0x18
8004fd80:  sw s0,0x10(sp)
8004fd84:  move s0,a0
8004fd88:  lui a1,0x8006
8004fd8c:  addiu a1,a1,0x5034
8004fd90:  sw ra,0x14(sp)
8004fd94:  jal 0x80044c44
8004fd98:  _li a2,0x5c
8004fd9c:  move v0,s0
8004fda0:  lw ra,0x14(sp)
8004fda4:  lw s0,0x10(sp)
8004fda8:  jr ra
8004fdac:  _addiu sp,sp,0x18
