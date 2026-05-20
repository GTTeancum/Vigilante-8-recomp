# addr: 0x80052694  name: PRNT_OBJ_50
80052694:  lbu a0,0x0(s7)
80052698:  nop
8005269c:  beq a0,zero,0x80052c74
800526a0:  _li v0,0x25
800526a4:  bne a0,v0,0x80052c98
800526a8:  _clear s3
800526ac:  li s4,-0x1
800526b0:  clear s8
800526b4:  sw zero,0x3c(sp)
800526b8:  sw zero,0x40(sp)
800526bc:  sw zero,0x44(sp)
