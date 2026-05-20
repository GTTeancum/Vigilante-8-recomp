# addr: 0x8001afa0  name: FUN_8001afa0
8001afa0:  andi v1,a1,0xffff
8001afa4:  ori v0,zero,0xffff
8001afa8:  beq v1,v0,0x8001afec
8001afac:  _andi a2,a2,0xffff
8001afb0:  move a3,v0
8001afb4:  andi v0,a1,0xffff
8001afb8:  sll v1,v0,0x3
8001afbc:  subu v1,v1,v0
8001afc0:  sll v1,v1,0x2
8001afc4:  addiu v1,v1,0x1c
8001afc8:  addu v1,a0,v1
8001afcc:  lhu v0,0x0(v1)
8001afd0:  nop
8001afd4:  beq v0,a2,0x8001aff4
8001afd8:  _move v0,v1
8001afdc:  lhu a1,0x18(v1)
8001afe0:  nop
8001afe4:  bne a1,a3,0x8001afb8
8001afe8:  _andi v0,a1,0xffff
8001afec:  jr ra
8001aff0:  _clear v0
8001aff4:  jr ra
8001aff8:  _nop
