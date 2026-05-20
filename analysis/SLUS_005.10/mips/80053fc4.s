# addr: 0x80053fc4  name: PadSetActAlign
80053fc4:  lui v0,0x8006
80053fc8:  lw v0,0x5270(v0)
80053fcc:  addiu sp,sp,-0x18
80053fd0:  sw s0,0x10(sp)
80053fd4:  sw ra,0x14(sp)
80053fd8:  jalr v0
80053fdc:  _move s0,a1
80053fe0:  move a0,v0
80053fe4:  jal 0x800552a8
80053fe8:  _move a1,s0
80053fec:  lw ra,0x14(sp)
80053ff0:  lw s0,0x10(sp)
80053ff4:  jr ra
80053ff8:  _addiu sp,sp,0x18
