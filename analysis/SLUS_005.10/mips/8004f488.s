# addr: 0x8004f488  name: DrawSyncCallback
8004f488:  addiu sp,sp,-0x20
8004f48c:  sw s0,0x10(sp)
8004f490:  lui s0,0x8006
8004f494:  addiu s0,s0,0x5026
8004f498:  sw ra,0x18(sp)
8004f49c:  sw s1,0x14(sp)
8004f4a0:  lbu v0,0x0(s0)
8004f4a4:  nop
8004f4a8:  sltiu v0,v0,0x2
8004f4ac:  bne v0,zero,0x8004f4cc
8004f4b0:  _move s1,a0
8004f4b4:  lui v0,0x8006
8004f4b8:  lw v0,0x5020(v0)
8004f4bc:  lui a0,0x8001
8004f4c0:  addiu a0,a0,0x120c
8004f4c4:  jalr v0
8004f4c8:  _move a1,s1
8004f4cc:  lw v0,0xa(s0)
8004f4d0:  sw s1,0xa(s0)
8004f4d4:  lw ra,0x18(sp)
8004f4d8:  lw s1,0x14(sp)
8004f4dc:  lw s0,0x10(sp)
8004f4e0:  jr ra
8004f4e4:  _addiu sp,sp,0x20
