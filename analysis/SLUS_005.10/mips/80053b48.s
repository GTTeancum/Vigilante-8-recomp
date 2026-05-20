# addr: 0x80053b48  name: StartRCnt
80053b48:  andi v0,a0,0xffff
80053b4c:  sll a0,v0,0x2
80053b50:  lui a1,0x8006
80053b54:  lw a1,0x5234(a1)
80053b58:  lui at,0x8006
80053b5c:  addu at,at,a0
80053b60:  lw a0,0x523c(at)
80053b64:  lw v1,0x4(a1)
80053b68:  slti v0,v0,0x3
80053b6c:  or v1,v1,a0
80053b70:  jr ra
80053b74:  _sw v1,0x4(a1)
