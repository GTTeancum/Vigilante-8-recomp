# addr: 0x800445b8  name: FUN_800445b8
800445b8:  addiu sp,sp,-0x18
800445bc:  sw ra,0x10(sp)
800445c0:  jal 0x80016a20
800445c4:  _nop
800445c8:  lh a0,0x8e4(gp)
800445cc:  lui v1,0x20
800445d0:  addu v0,v0,v1
800445d4:  bgez v0,0x800445e0
800445d8:  _sll v1,a0,0x9
800445dc:  addiu v0,v0,0xfff
800445e0:  sra v0,v0,0xc
800445e4:  div v1,v0
800445e8:  mflo v0
800445ec:  lw ra,0x10(sp)
800445f0:  sll v0,v0,0x10
800445f4:  sra v0,v0,0x10
800445f8:  jr ra
800445fc:  _addiu sp,sp,0x18
