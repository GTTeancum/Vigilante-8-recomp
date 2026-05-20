# addr: 0x8004a9f4  name: BIOS_OBJ_1440
8004a9f4:  bne v0,zero,0x8004aa24
8004a9f8:  _li v0,-0x1
8004a9fc:  lui v0,0x8006
8004aa00:  lw v0,0x388(v0)
8004aa04:  nop
8004aa08:  lw v0,0x0(v0)
8004aa0c:  lui v1,0x100
8004aa10:  and v0,v0,v1
8004aa14:  beq v0,zero,0x8004aa24
8004aa18:  _clear v0
8004aa1c:  beq s2,zero,0x8004a944
8004aa20:  _li v0,0x1
8004aa24:  lw ra,0x2c(sp)
8004aa28:  lw s4,0x28(sp)
8004aa2c:  lw s3,0x24(sp)
8004aa30:  lw s2,0x20(sp)
8004aa34:  lw s1,0x1c(sp)
8004aa38:  lw s0,0x18(sp)
8004aa3c:  jr ra
8004aa40:  _addiu sp,sp,0x30
