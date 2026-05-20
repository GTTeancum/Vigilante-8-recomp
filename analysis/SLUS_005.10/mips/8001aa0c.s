# addr: 0x8001aa0c  name: FUN_8001aa0c
8001aa0c:  addiu sp,sp,-0x18
8001aa10:  sw s0,0x10(sp)
8001aa14:  sw ra,0x14(sp)
8001aa18:  jal 0x8001a994
8001aa1c:  _move s0,a0
8001aa20:  jal 0x80045088
8001aa24:  _move a0,s0
8001aa28:  lw ra,0x14(sp)
8001aa2c:  lw s0,0x10(sp)
8001aa30:  jr ra
8001aa34:  _addiu sp,sp,0x18
