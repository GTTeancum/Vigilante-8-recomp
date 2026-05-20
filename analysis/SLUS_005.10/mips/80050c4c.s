# addr: 0x80050c4c  name: SYS_OBJ_1A48
80050c4c:  lh v1,0x6(t0)
80050c50:  lhu a0,0x6(t0)
80050c54:  bltz v1,0x80050c88
80050c58:  _sh v0,0x4(t0)
80050c5c:  lui v0,0x8006
80050c60:  lh v0,0x502a(v0)
80050c64:  nop
80050c68:  addiu v0,v0,-0x1
80050c6c:  slt v0,v0,v1
80050c70:  lui v1,0x8006
80050c74:  lhu v1,0x502a(v1)
80050c78:  bne v0,zero,0x80050c8c
80050c7c:  _addiu v1,v1,-0x1
80050c80:  j 0x80050c8c
80050c84:  _move v1,a0
80050c88:  clear v1
