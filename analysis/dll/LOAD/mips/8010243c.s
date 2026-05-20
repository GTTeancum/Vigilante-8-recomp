# addr: 0x8010243c  name: FUN_8010243c
8010243c:  addiu sp,sp,-0x18
80102440:  sw s0,0x10(sp)
80102444:  move s0,a0
80102448:  sll a0,s0,0x4
8010244c:  sw ra,0x14(sp)
80102450:  jal 0x800116f4
80102454:  _addiu a0,a0,0x30
80102458:  sw s0,0x1c(v0)
8010245c:  lw ra,0x14(sp)
80102460:  lw s0,0x10(sp)
80102464:  jr ra
80102468:  _addiu sp,sp,0x18
