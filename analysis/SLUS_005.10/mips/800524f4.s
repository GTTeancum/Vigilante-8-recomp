# addr: 0x800524f4  name: strcpy
800524f4:  beq a0,zero,0x80052530
800524f8:  _clear v0
800524fc:  beq a1,zero,0x80052530
80052500:  _move v1,a0
80052504:  lbu v0,0x0(a1)
80052508:  addiu a1,a1,0x1
8005250c:  addiu a0,v1,0x1
80052510:  beq v0,zero,0x8005252c
80052514:  _sb v0,0x0(v1)
80052518:  lbu v0,0x0(a1)
8005251c:  addiu a1,a1,0x1
80052520:  sb v0,0x0(a0)
80052524:  bne v0,zero,0x80052518
80052528:  _addiu a0,a0,0x1
8005252c:  move v0,v1
80052530:  jr ra
80052534:  _nop
