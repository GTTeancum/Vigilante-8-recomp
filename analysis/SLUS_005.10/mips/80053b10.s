# addr: 0x80053b10  name: GetRCnt
80053b10:  andi v1,a0,0xffff
80053b14:  slti v0,v1,0x3
80053b18:  beq v0,zero,0x80053b3c
80053b1c:  _sll v1,v1,0x4
80053b20:  lui v0,0x8006
80053b24:  lw v0,0x5238(v0)
80053b28:  nop
80053b2c:  addu v1,v1,v0
80053b30:  lhu v0,0x0(v1)
80053b34:  j 0x80053b40
80053b38:  _nop
80053b3c:  clear v0
