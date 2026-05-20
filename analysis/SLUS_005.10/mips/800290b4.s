# addr: 0x800290b4  name: FUN_800290b4
800290b4:  lui at,0x1f80
800290b8:  sw ra,0x3f0(at)
800290bc:  sw sp,0x3f4(at)
800290c0:  jal 0x800290d8
800290c4:  _addi sp,at,0x3e0
800290c8:  lw ra,0x10(sp)
800290cc:  lw sp,0x14(sp)
800290d0:  jr ra
800290d4:  _nop
