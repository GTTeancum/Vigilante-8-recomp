# addr: 0x80052604  name: printf
80052604:  addiu sp,sp,-0x18
80052608:  addiu v0,sp,0x1c
8005260c:  sw a0,0x18(sp)
80052610:  li a0,0x1
80052614:  sw ra,0x10(sp)
80052618:  sw a1,0x1c(sp)
8005261c:  sw a2,0x4(v0)
80052620:  sw a3,0x8(v0)
80052624:  lw a1,0x18(sp)
80052628:  jal 0x80052644
8005262c:  _move a2,v0
80052630:  lw ra,0x10(sp)
80052634:  addiu sp,sp,0x18
80052638:  jr ra
8005263c:  _nop
