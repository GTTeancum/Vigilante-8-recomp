# addr: 0x80044054  name: FUN_80044054
80044054:  addiu sp,sp,-0x18
80044058:  lui a1,0xff
8004405c:  ori a1,a1,0xffff
80044060:  sw ra,0x10(sp)
80044064:  sw zero,0x8fc(gp)
80044068:  jal 0x80043ff0
8004406c:  _clear a0
80044070:  lw ra,0x10(sp)
80044074:  nop
80044078:  jr ra
8004407c:  _addiu sp,sp,0x18
