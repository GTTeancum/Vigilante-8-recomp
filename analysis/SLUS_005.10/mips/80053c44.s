# addr: 0x80053c44  name: PadChkMtap
80053c44:  lui v0,0x8006
80053c48:  lw v0,0x52a8(v0)
80053c4c:  nop
80053c50:  bne v0,zero,0x80053c60
80053c54:  _sra v1,a0,0x4
80053c58:  j 0x80053c88
80053c5c:  _clear v0
80053c60:  sll v0,v1,0x4
80053c64:  subu v0,v0,v1
80053c68:  lui v1,0x8006
80053c6c:  lw v1,0x5290(v1)
80053c70:  sll v0,v0,0x4
80053c74:  addu v0,v0,v1
80053c78:  lbu v0,0xe8(v0)
80053c7c:  nop
80053c80:  xori v0,v0,0x8
80053c84:  sltiu v0,v0,0x1
