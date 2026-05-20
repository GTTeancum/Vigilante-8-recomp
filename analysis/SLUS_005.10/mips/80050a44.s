# addr: 0x80050a44  name: SYS_OBJ_1840
80050a44:  andi v1,a1,0x3ff
80050a48:  sll v1,v1,0xa
80050a4c:  andi v0,a0,0x3ff
80050a50:  lui a0,0xe400
80050a54:  or v0,v0,a0
80050a58:  jr ra
80050a5c:  _or v0,v1,v0
