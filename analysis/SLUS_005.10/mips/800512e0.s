# addr: 0x800512e0  name: SYS_OBJ_20DC
800512e0:  lui v0,0x8006
800512e4:  lw v0,0x512c(v0)
800512e8:  nop
800512ec:  sw a0,0x0(v0)
800512f0:  srl v0,a0,0x18
800512f4:  lui at,0x800a
800512f8:  addu at,at,v0
800512fc:  jr ra
80051300:  _sb a0,0x3340(at)
