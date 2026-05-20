# addr: 0x80053bac  name: ResetRCnt
80053bac:  andi v1,a0,0xffff
80053bb0:  slti v0,v1,0x3
80053bb4:  beq v0,zero,0x80053bd4
80053bb8:  _li v0,0x1
80053bbc:  lui a0,0x8006
80053bc0:  lw a0,0x5238(a0)
80053bc4:  sll v1,v1,0x4
80053bc8:  addu v1,v1,a0
80053bcc:  j 0x80053bd8
80053bd0:  _sh zero,0x0(v1)
80053bd4:  clear v0
