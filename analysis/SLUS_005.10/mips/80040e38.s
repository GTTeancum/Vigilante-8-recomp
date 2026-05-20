# addr: 0x80040e38  name: FUN_80040e38
80040e38:  lui at,0x1f80
80040e3c:  sw ra,0x3f0(at)
80040e40:  sw sp,0x3f4(at)
80040e44:  jal 0x80040e5c
80040e48:  _addi sp,at,0x3e0
80040e4c:  lw ra,0x10(sp)
80040e50:  lw sp,0x14(sp)
80040e54:  jr ra
80040e58:  _nop
