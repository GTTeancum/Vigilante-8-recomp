# addr: 0x800522b4  name: SetDrawTPage
800522b4:  li v0,0x1
800522b8:  sb v0,0x3(a0)
800522bc:  beq a2,zero,0x800522c8
800522c0:  _lui v1,0xe100
800522c4:  ori v1,v1,0x200
800522c8:  beq a1,zero,0x800522d4
800522cc:  _andi v0,a3,0x9ff
800522d0:  ori v0,v0,0x400
800522d4:  or v0,v1,v0
800522d8:  jr ra
800522dc:  _sw v0,0x4(a0)
