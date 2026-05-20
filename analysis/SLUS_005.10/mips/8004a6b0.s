# addr: 0x8004a6b0  name: CD_initintr
8004a6b0:  addiu sp,sp,-0x18
8004a6b4:  sw ra,0x10(sp)
8004a6b8:  lui at,0x8006
8004a6bc:  sw zero,0x80(at)
8004a6c0:  lui at,0x8006
8004a6c4:  sw zero,0x7c(at)
8004a6c8:  lui at,0x8006
8004a6cc:  sw zero,0x90(at)
8004a6d0:  lui at,0x8006
8004a6d4:  jal 0x80048054
8004a6d8:  _sw zero,0x8c(at)
8004a6dc:  lui a1,0x8005
8004a6e0:  addiu a1,a1,-0x53c4
8004a6e4:  jal 0x80048084
8004a6e8:  _li a0,0x2
8004a6ec:  lw ra,0x10(sp)
8004a6f0:  addiu sp,sp,0x18
8004a6f4:  jr ra
8004a6f8:  _nop
