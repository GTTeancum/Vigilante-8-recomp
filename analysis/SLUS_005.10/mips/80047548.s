# addr: 0x80047548  name: _spu_pitch2note
80047548:  move t8,a0
8004754c:  move a0,a2
80047550:  nor a2,zero,a2
80047554:  clear v1
80047558:  li t2,0xf
8004755c:  andi a2,a2,0xffff
80047560:  srav v0,a2,t2
80047564:  andi v0,v0,0x1
80047568:  bne v0,zero,0x80047578
8004756c:  _nop
80047570:  j 0x80047584
80047574:  _move v1,t2
80047578:  addiu t2,t2,-0x1
8004757c:  bgez t2,0x80047564
80047580:  _srav v0,a2,t2
