# addr: 0x80053360  name: SPRINTF_OBJ_35C
80053360:  lw v0,0x220(sp)
80053364:  nop
80053368:  lw a0,0x0(v0)
8005336c:  addiu v0,v0,0x4
80053370:  sw v0,0x220(sp)
80053374:  lw v0,0x210(sp)
80053378:  nop
8005337c:  srl v0,v0,0x5
80053380:  andi v0,v0,0x1
80053384:  beq v0,zero,0x80053390
80053388:  _nop
8005338c:  andi a0,a0,0xffff
80053390:  sb zero,0x211(sp)
