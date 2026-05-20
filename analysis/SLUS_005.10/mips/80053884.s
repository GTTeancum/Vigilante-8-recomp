# addr: 0x80053884  name: puts
80053884:  addiu sp,sp,-0x18
80053888:  sw s0,0x10(sp)
8005388c:  move s0,a0
80053890:  bne s0,zero,0x800538ac
80053894:  _sw ra,0x14(sp)
80053898:  lui s0,0x8001
8005389c:  j 0x800538ac
800538a0:  _addiu s0,s0,0x1684
800538a4:  jal 0x80052da4
800538a8:  _sra a0,a0,0x18
