# addr: 0x80053b78  name: StopRCnt
80053b78:  andi a0,a0,0xffff
80053b7c:  sll a0,a0,0x2
80053b80:  lui a1,0x8006
80053b84:  lw a1,0x5234(a1)
80053b88:  lui v0,0x8006
80053b8c:  addu v0,v0,a0
80053b90:  lw v0,0x523c(v0)
80053b94:  lw v1,0x4(a1)
80053b98:  nor v0,zero,v0
80053b9c:  and v1,v1,v0
80053ba0:  li v0,0x1
80053ba4:  jr ra
80053ba8:  _sw v1,0x4(a1)
