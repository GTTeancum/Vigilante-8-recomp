# addr: 0x800513a0  name: SYS_OBJ_219C
800513a0:  lui v0,0x1000
800513a4:  lui v1,0x8006
800513a8:  lw v1,0x512c(v1)
800513ac:  or a0,a0,v0
800513b0:  sw a0,0x0(v1)
800513b4:  lui v0,0x8006
800513b8:  lw v0,0x5128(v0)
800513bc:  lui v1,0xff
800513c0:  lw v0,0x0(v0)
800513c4:  ori v1,v1,0xffff
800513c8:  jr ra
800513cc:  _and v0,v0,v1
