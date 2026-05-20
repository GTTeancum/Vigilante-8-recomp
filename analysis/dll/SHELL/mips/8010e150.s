# addr: 0x8010e150  name: FUN_8010e150
8010e150:  addiu sp,sp,-0x18
8010e154:  sw s0,0x10(sp)
8010e158:  move s0,a0
8010e15c:  bne s0,zero,0x8010e16c
8010e160:  _sw ra,0x14(sp)
8010e164:  jal 0x80048054
8010e168:  _nop
8010e16c:  sltiu a0,zero,0x3904
8010e170:  move a0,s0
8010e174:  lw ra,0x14(sp)
8010e178:  lw s0,0x10(sp)
8010e17c:  jr ra
8010e180:  _addiu sp,sp,0x18
