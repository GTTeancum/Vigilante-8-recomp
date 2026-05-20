# addr: 0x800467b4  name: SpuSetTransferMode
800467b4:  beq a0,zero,0x800467cc
800467b8:  _li v0,0x1
800467bc:  bne a0,v0,0x800467d0
800467c0:  _clear v0
800467c4:  j 0x800467d0
800467c8:  _li v0,0x1
800467cc:  clear v0
