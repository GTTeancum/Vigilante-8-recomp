# addr: 0x80106234  name: FUN_80106234
80106234:  addiu sp,sp,-0x18
80106238:  sw s0,0x10(sp)
8010623c:  move s0,a0
80106240:  bne s0,zero,0x80106250
80106244:  _sw ra,0x14(sp)
80106248:  jal 0x80048054
8010624c:  _nop
80106250:  sltiu a0,zero,0x193d
80106254:  move a0,s0
80106258:  lw ra,0x14(sp)
8010625c:  lw s0,0x10(sp)
80106260:  jr ra
80106264:  _addiu sp,sp,0x18
