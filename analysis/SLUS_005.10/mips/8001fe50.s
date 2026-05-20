# addr: 0x8001fe50  name: FUN_8001fe50
8001fe50:  lw v0,0x76c(gp)
8001fe54:  nop
8001fe58:  lw a2,0x0(v0)
8001fe5c:  lui v1,0x8006
8001fe60:  addiu v1,v1,0x5a70
8001fe64:  sw v1,0x4(a2)
8001fe68:  sw a2,0x76c(gp)
8001fe6c:  sw a1,0x8(v0)
8001fe70:  lw v1,0x8(a0)
8001fe74:  sw v0,0x8(a0)
8001fe78:  addiu a0,a0,0x4
8001fe7c:  sw v0,0x0(v1)
8001fe80:  sw v1,0x4(v0)
8001fe84:  jr ra
8001fe88:  _sw a0,0x0(v0)
