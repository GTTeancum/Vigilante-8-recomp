# addr: 0x800566b4  name: FUN_800566b4
800566b4:  lui v0,0x1f80
800566b8:  ori v0,v0,0x1120
800566bc:  lhu v0,0x0(v0)
800566c0:  lui at,0x800a
800566c4:  sw a0,0x4f0c(at)
800566c8:  lui at,0x800a
800566cc:  jr ra
800566d0:  _sw v0,0x4f08(at)
