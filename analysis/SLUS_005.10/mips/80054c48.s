# addr: 0x80054c48  name: FUN_80054c48
80054c48:  lui v1,0x8006
80054c4c:  lw v1,0x52c0(v1)
80054c50:  nop
80054c54:  lhu v0,0x4(v1)
80054c58:  nop
80054c5c:  andi v0,v0,0x2
80054c60:  beq v0,zero,0x80054c54
80054c64:  _nop
80054c68:  jr ra
80054c6c:  _nop
