# addr: 0x8004ceb4  name: ReadRotMatrix
8004ceb4:  gte_stR11R12 t0
8004ceb8:  gte_stR13R21 t1
8004cebc:  gte_stR22R23 t2
8004cec0:  gte_stR31R32 t3
8004cec4:  gte_stR33 t4
8004cec8:  sw t0,0x0(a0)
8004cecc:  sw t1,0x4(a0)
8004ced0:  sw t2,0x8(a0)
8004ced4:  sw t3,0xc(a0)
8004ced8:  sw t4,0x10(a0)
8004cedc:  gte_stTRX t0
8004cee0:  gte_stTRY t1
8004cee4:  gte_stTRZ t2
8004cee8:  sw t0,0x14(a0)
8004ceec:  sw t1,0x18(a0)
8004cef0:  sw t2,0x1c(a0)
8004cef4:  jr ra
8004cef8:  _nop
