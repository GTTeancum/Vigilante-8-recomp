# addr: 0x8002b940  name: FUN_8002b940
8002b940:  lh v1,0x6(a0)
8002b944:  li v0,-0x1
8002b948:  beq v1,v0,0x8002b984
8002b94c:  _lui v0,0x8006
8002b950:  li v0,-0x2
8002b954:  beq v1,v0,0x8002b97c
8002b958:  _lui v0,0x8006
8002b95c:  lbu v1,0xd0(a0)
8002b960:  lui v0,0x8005
8002b964:  addiu v0,v0,0x67ec
8002b968:  sll v1,v1,0x3
8002b96c:  addu v1,v1,v0
8002b970:  lw v0,0x4(v1)
8002b974:  jr ra
8002b978:  _nop
8002b97c:  jr ra
8002b980:  _addiu v0,v0,0x56f8
8002b984:  jr ra
8002b988:  _addiu v0,v0,0x5704
