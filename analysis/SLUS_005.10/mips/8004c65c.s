# addr: 0x8004c65c  name: InitGeom
8004c65c:  lui at,0x8006
8004c660:  sw ra,0x484(at)
8004c664:  jal 0x8004ee54
8004c668:  _nop
8004c66c:  lui ra,0x8006
8004c670:  lw ra,0x484(ra)
8004c674:  nop
8004c678:  mfc0 v0,Status
8004c67c:  lui v1,0x4000
8004c680:  or v0,v0,v1
8004c684:  mtc0 v0,Status,0x0
8004c688:  nop
8004c68c:  li t0,0x155
8004c690:  gte_ldZSF3 t0
8004c694:  nop
8004c698:  li t0,0x100
8004c69c:  gte_ldZSF4 t0
8004c6a0:  nop
8004c6a4:  li t0,0x3e8
8004c6a8:  gte_ldH t0
8004c6ac:  nop
8004c6b0:  li t0,-0x1062
8004c6b4:  gte_ldDQA t0
8004c6b8:  nop
8004c6bc:  lui t0,0x140
8004c6c0:  gte_ldDQB t0
8004c6c4:  nop
8004c6c8:  gte_ldOFX zero
8004c6cc:  gte_ldOFY zero
8004c6d0:  nop
8004c6d4:  jr ra
8004c6d8:  _nop
