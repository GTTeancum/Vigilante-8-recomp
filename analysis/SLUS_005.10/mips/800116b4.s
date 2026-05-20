# addr: 0x800116b4  name: start
800116b4:  lui sp,0x801f
800116b8:  ori sp,sp,0xfff0
800116bc:  lui v0,0x8006
800116c0:  addiu v0,v0,0x58d8
800116c4:  lui v1,0x800a
800116c8:  addiu v1,v1,0x4f10
800116cc:  sw zero,0x0(v0)
800116d0:  sltu at,v0,v1
800116d4:  bne at,zero,0x800116cc
800116d8:  _addiu v0,v0,0x4
800116dc:  lui gp,0x8006
800116e0:  addiu gp,gp,0x5304
800116e4:  j 0x80015098
800116e8:  _nop
