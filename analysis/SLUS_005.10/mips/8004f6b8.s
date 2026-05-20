# addr: 0x8004f6b8  name: SYS_OBJ_4B4
8004f6b8:  lui v0,0x8006
8004f6bc:  lw v0,0x5020(v0)
8004f6c0:  nop
8004f6c4:  jalr v0
8004f6c8:  _move a1,t0
8004f6cc:  lh a1,0x0(s0)
8004f6d0:  lh a2,0x2(s0)
8004f6d4:  lh a3,0x4(s0)
8004f6d8:  lh v1,0x6(s0)
8004f6dc:  lui v0,0x8006
8004f6e0:  lw v0,0x5020(v0)
8004f6e4:  lui a0,0x8001
8004f6e8:  addiu a0,a0,0x125c
8004f6ec:  jalr v0
8004f6f0:  _sw v1,0x10(sp)
