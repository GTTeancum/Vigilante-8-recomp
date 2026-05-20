# addr: 0x80018cb8  name: FUN_80018cb8
80018cb8:  lhu v0,0x8(a1)
80018cbc:  lui v1,0xe100
80018cc0:  ori v1,v1,0x400
80018cc4:  andi v0,v0,0x9ff
80018cc8:  or v0,v0,v1
80018ccc:  sw v0,0x4(a0)
80018cd0:  lhu v0,0xa(a1)
80018cd4:  nop
80018cd8:  sh v0,0x16(a0)
80018cdc:  lhu v0,0x2(a1)
80018ce0:  nop
80018ce4:  sh v0,0x18(a0)
80018ce8:  lhu v0,0x4(a1)
80018cec:  nop
80018cf0:  sh v0,0x1a(a0)
80018cf4:  lhu v0,0x6(a1)
80018cf8:  jr ra
80018cfc:  _sh v0,0x14(a0)
