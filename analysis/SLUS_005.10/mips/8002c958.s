# addr: 0x8002c958  name: FUN_8002c958
8002c958:  addiu sp,sp,-0x18
8002c95c:  sw ra,0x10(sp)
8002c960:  lhu v0,0x11e(a0)
8002c964:  nop
8002c968:  bne v0,zero,0x8002c980
8002c96c:  _nop
8002c970:  jal 0x8002c6fc
8002c974:  _nop
8002c978:  j 0x8002c98c
8002c97c:  _nop
8002c980:  jal 0x8002c018
8002c984:  _nop
8002c988:  clear v0
8002c98c:  lw ra,0x10(sp)
8002c990:  nop
8002c994:  jr ra
8002c998:  _addiu sp,sp,0x18
