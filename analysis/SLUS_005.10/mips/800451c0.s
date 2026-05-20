# addr: 0x800451c0  name: FUN_800451c0
800451c0:  mult a1,a0
800451c4:  move t9,ra
800451c8:  mflo t8
800451cc:  jal 0x80045004
800451d0:  _move a0,t8
800451d4:  beq v0,zero,0x800451f0
800451d8:  _move a0,v0
800451dc:  sw zero,0x0(a0)
800451e0:  sw zero,0x4(a0)
800451e4:  addi t8,t8,-0x8
800451e8:  bgtz t8,0x800451dc
800451ec:  _addi a0,a0,0x8
800451f0:  jr t9
800451f4:  _nop
