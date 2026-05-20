# addr: 0x80050a60  name: SYS_OBJ_185C
80050a60:  andi a1,a1,0x7ff
80050a64:  sll a1,a1,0xb
80050a68:  andi v0,a0,0x7ff
80050a6c:  lui v1,0xe500
80050a70:  or v0,v0,v1
80050a74:  jr ra
80050a78:  _or v0,a1,v0
