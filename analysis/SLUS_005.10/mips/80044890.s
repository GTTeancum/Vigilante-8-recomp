# addr: 0x80044890  name: FUN_80044890
80044890:  addiu sp,sp,-0x18
80044894:  sw ra,0x10(sp)
80044898:  jal 0x80016a20
8004489c:  _nop
800448a0:  lui v1,0x20
800448a4:  subu v0,v1,v0
800448a8:  bltz v0,0x800448d8
800448ac:  _sra v0,v0,0xc
800448b0:  lh v1,0x8e4(gp)
800448b4:  nop
800448b8:  mult v0,v1
800448bc:  mflo v0
800448c0:  bgez v0,0x800448cc
800448c4:  _nop
800448c8:  addiu v0,v0,0x1ff
800448cc:  sll v0,v0,0x7
800448d0:  j 0x800448dc
800448d4:  _sra v0,v0,0x10
800448d8:  clear v0
800448dc:  lw ra,0x10(sp)
800448e0:  nop
800448e4:  jr ra
800448e8:  _addiu sp,sp,0x18
