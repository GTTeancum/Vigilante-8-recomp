# addr: 0x8004e7d0  name: DIVG3A_OBJ_3AC
8004e7d0:  lb t8,0x13(a1)
8004e7d4:  lw t4,0x10(t0)
8004e7d8:  sb t8,0xf(t0)
8004e7dc:  lw t5,0x10(t1)
8004e7e0:  lw t6,0x10(t2)
8004e7e4:  sw t4,0x8(a0)
8004e7e8:  sw t5,0x10(a0)
8004e7ec:  sw t6,0x18(a0)
8004e7f0:  lw t4,0x14(a1)
8004e7f4:  sll t5,a0,0x8
8004e7f8:  srl t5,t5,0x8
8004e7fc:  lw t7,0x0(t4)
8004e800:  sw t5,0x0(t4)
8004e804:  lui t6,0x600
8004e808:  or t7,t7,t6
8004e80c:  lw t4,0xc(t0)
8004e810:  lw t5,0xc(t1)
8004e814:  lw t6,0xc(t2)
8004e818:  sw t4,0x4(a0)
8004e81c:  sw t5,0xc(a0)
8004e820:  sw t6,0x14(a0)
8004e824:  sw t7,0x0(a0)
8004e828:  addiu a0,a0,0x1c
8004e82c:  jr ra
8004e830:  _nop
