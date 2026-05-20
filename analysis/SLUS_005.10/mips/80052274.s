# addr: 0x80052274  name: AddPrim
80052274:  lui a2,0xff
80052278:  ori a2,a2,0xffff
8005227c:  lui a3,0xff00
80052280:  lw v1,0x0(a1)
80052284:  lw v0,0x0(a0)
80052288:  and v1,v1,a3
8005228c:  and v0,v0,a2
80052290:  or v1,v1,v0
80052294:  sw v1,0x0(a1)
80052298:  lw v0,0x0(a0)
8005229c:  and a1,a1,a2
800522a0:  and v0,v0,a3
800522a4:  or v0,v0,a1
800522a8:  jr ra
800522ac:  _sw v0,0x0(a0)
