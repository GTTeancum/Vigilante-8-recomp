# addr: 0x800531c0  name: SPRINTF_OBJ_1BC
800531c0:  bne a1,v0,0x8005327c
800531c4:  _nop
800531c8:  lw a0,0x24c(sp)
800531cc:  nop
800531d0:  addiu v0,a0,0x1
800531d4:  sw v0,0x24c(sp)
800531d8:  lb a1,0x1(a0)
800531dc:  li v0,0x2a
800531e0:  bne a1,v0,0x80053250
800531e4:  _addiu v0,a1,-0x30
800531e8:  lw v0,0x220(sp)
800531ec:  nop
800531f0:  lw v1,0x0(v0)
800531f4:  addiu v0,v0,0x4
800531f8:  sw v0,0x220(sp)
800531fc:  addiu v0,a0,0x2
80053200:  sw v1,0x218(sp)
80053204:  sw v0,0x24c(sp)
80053208:  lb a1,0x2(a0)
8005320c:  j 0x8005325c
80053210:  _nop
80053214:  lw v1,0x218(sp)
80053218:  nop
8005321c:  sll v0,v1,0x2
80053220:  addu v0,v0,v1
80053224:  sll v0,v0,0x1
80053228:  addiu v0,v0,-0x30
8005322c:  addu v0,v0,a1
80053230:  sw v0,0x218(sp)
80053234:  lw v1,0x24c(sp)
80053238:  nop
8005323c:  addiu v0,v1,0x1
80053240:  sw v0,0x24c(sp)
80053244:  lb a1,0x1(v1)
80053248:  nop
8005324c:  addiu v0,a1,-0x30
80053250:  sltiu v0,v0,0xa
80053254:  bne v0,zero,0x80053214
80053258:  _nop
