# addr: 0x80050e8c  name: SYS_OBJ_1C88
80050e8c:  lh a1,0x6(s1)
80050e90:  lhu v1,0x6(s1)
80050e94:  bltz a1,0x80050ec4
80050e98:  _sh a0,0x4(s1)
80050e9c:  move a0,v1
80050ea0:  lui v0,0x8006
80050ea4:  lh v0,0x502a(v0)
80050ea8:  lui v1,0x8006
80050eac:  lhu v1,0x502a(v1)
80050eb0:  slt v0,v0,a1
80050eb4:  beq v0,zero,0x80050ecc
80050eb8:  _sll v0,a0,0x10
80050ebc:  j 0x80050ec8
80050ec0:  _move a0,v1
80050ec4:  clear a0
