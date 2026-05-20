# addr: 0x8004efe0  name: EXT_OBJ_AC
8004efe0:  jal 0x8004f82c
8004efe4:  _move a1,t0
8004efe8:  move a0,s0
8004efec:  move a1,s2
8004eff0:  move a2,s1
8004eff4:  jal 0x80052214
8004eff8:  _move a3,s3
8004effc:  andi v0,v0,0xffff
8004f000:  lw ra,0x28(sp)
8004f004:  lw s3,0x24(sp)
8004f008:  lw s2,0x20(sp)
8004f00c:  lw s1,0x1c(sp)
8004f010:  lw s0,0x18(sp)
8004f014:  jr ra
8004f018:  _addiu sp,sp,0x30
