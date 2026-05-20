# addr: 0x8001265c  name: FUN_8001265c
8001265c:  lw v1,0x618(gp)
80012660:  li v0,0x2
80012664:  bne v1,v0,0x80012678
80012668:  _ori v0,zero,0xffff
8001266c:  lw v1,0x610(gp)
80012670:  nop
80012674:  sh v0,0x0(v1)
80012678:  sw zero,0x618(gp)
8001267c:  jr ra
80012680:  _nop
