# addr: 0x8003d1e8  name: FUN_8003d1e8
8003d1e8:  sltiu v0,a0,0xd
8003d1ec:  beq v0,zero,0x8003d20c
8003d1f0:  _lui v1,0x8006
8003d1f4:  addiu v1,v1,-0x1350
8003d1f8:  sll v0,a0,0x2
8003d1fc:  addu v0,v0,v1
8003d200:  lw v0,0x0(v0)
8003d204:  jr ra
8003d208:  _nop
8003d20c:  jr ra
8003d210:  _clear v0
