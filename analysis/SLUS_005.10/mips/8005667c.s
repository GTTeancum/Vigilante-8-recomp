# addr: 0x8005667c  name: PADSEQD_OBJ_448
8005667c:  lhu v0,0xe6(a0)
80056680:  nop
80056684:  beq v0,zero,0x8005669c
80056688:  _li v0,0xff
8005668c:  lbu v1,0x46(a0)
80056690:  nop
80056694:  beq v1,v0,0x800566a0
80056698:  _clear v0
8005669c:  li v0,0x1
800566a0:  jr ra
800566a4:  _nop
