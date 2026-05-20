# addr: 0x801061c4  name: FUN_801061c4
801061c4:  lui a2,0xff
801061c8:  ori a2,a2,0xffff
801061cc:  lui v1,0xff00
801061d0:  lw v0,0x0(a0)
801061d4:  and a1,a1,a2
801061d8:  and v0,v0,v1
801061dc:  or v0,v0,a1
801061e0:  jr ra
801061e4:  _sw v0,0x0(a0)
