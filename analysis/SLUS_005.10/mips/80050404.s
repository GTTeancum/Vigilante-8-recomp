# addr: 0x80050404  name: SetPriority
80050404:  li v0,0x2
80050408:  sb v0,0x3(a0)
8005040c:  beq a1,zero,0x80050418
80050410:  _lui v1,0xe600
80050414:  ori v1,v1,0x2
80050418:  sltu v0,zero,a2
8005041c:  or v0,v1,v0
80050420:  sw v0,0x4(a0)
80050424:  jr ra
80050428:  _sw zero,0x8(a0)
