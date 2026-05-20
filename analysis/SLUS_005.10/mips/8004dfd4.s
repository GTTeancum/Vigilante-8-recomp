# addr: 0x8004dfd4  name: DIVF3A_OBJ_320
8004dfd4:  lw t8,0x10(a1)
8004dfd8:  lw t4,0x10(t0)
8004dfdc:  lw t5,0x10(t1)
8004dfe0:  lw t6,0x10(t2)
8004dfe4:  sw t4,0x8(a0)
8004dfe8:  sw t5,0xc(a0)
8004dfec:  sw t6,0x10(a0)
8004dff0:  sw t8,0x4(a0)
8004dff4:  lw t4,0x14(a1)
8004dff8:  sll t9,a0,0x8
8004dffc:  srl t9,t9,0x8
8004e000:  lw t8,0x0(t4)
8004e004:  sw t9,0x0(t4)
8004e008:  lui t6,0x400
8004e00c:  or t8,t8,t6
8004e010:  sw t8,0x0(a0)
8004e014:  addiu a0,a0,0x14
8004e018:  jr ra
8004e01c:  _nop
