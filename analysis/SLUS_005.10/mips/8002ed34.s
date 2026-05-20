# addr: 0x8002ed34  name: FUN_8002ed34
8002ed34:  addiu sp,sp,-0x38
8002ed38:  sw s8,0x30(sp)
8002ed3c:  move s8,a0
8002ed40:  sw s6,0x28(sp)
8002ed44:  clear s6
8002ed48:  sw s5,0x24(sp)
8002ed4c:  move s5,s6
8002ed50:  sw s3,0x1c(sp)
8002ed54:  li s3,-0x1
8002ed58:  sw s2,0x18(sp)
8002ed5c:  move s2,s3
8002ed60:  sw ra,0x34(sp)
8002ed64:  sw s7,0x2c(sp)
8002ed68:  sw s4,0x20(sp)
8002ed6c:  sw s1,0x14(sp)
8002ed70:  beq a1,zero,0x8002ed8c
8002ed74:  _sw s0,0x10(sp)
8002ed78:  addiu a0,s8,0x24
8002ed7c:  jal 0x80016aac
8002ed80:  _addiu a1,a1,0x48
8002ed84:  j 0x8002ed90
8002ed88:  _move s4,v0
8002ed8c:  clear s4
8002ed90:  lw v0,0x714(gp)
8002ed94:  nop
8002ed98:  lw s1,0x0(v0)
8002ed9c:  nop
8002eda0:  beq s1,zero,0x8002ee54
8002eda4:  _li s7,0x3
8002eda8:  lw s0,0x8(v0)
8002edac:  nop
8002edb0:  beq s0,s8,0x8002ee44
8002edb4:  _move v0,s1
8002edb8:  lbu v0,0x4(s0)
8002edbc:  nop
8002edc0:  beq v0,s7,0x8002ee44
8002edc4:  _move v0,s1
8002edc8:  lw v0,0x0(s0)
8002edcc:  nop
8002edd0:  andi v0,v0,0x4000
8002edd4:  beq v0,zero,0x8002ee44
8002edd8:  _move v0,s1
8002eddc:  lh v0,0x6(s0)
8002ede0:  nop
8002ede4:  bgtz v0,0x8002ee00
8002ede8:  _addiu a0,s8,0x24
8002edec:  lb v0,0x15(gp)
8002edf0:  nop
8002edf4:  bne v0,s7,0x8002ee44
8002edf8:  _move v0,s1
8002edfc:  addiu a0,s8,0x24
8002ee00:  jal 0x80016aac
8002ee04:  _addiu a1,s0,0x48
8002ee08:  move v1,v0
8002ee0c:  sltu v0,s4,v1
8002ee10:  beq v0,zero,0x8002ee2c
8002ee14:  _sltu v0,v1,s2
8002ee18:  beq v0,zero,0x8002ee30
8002ee1c:  _sltu v0,v1,s3
8002ee20:  move s5,s0
8002ee24:  j 0x8002ee40
8002ee28:  _move s2,v1
8002ee2c:  sltu v0,v1,s3
8002ee30:  beq v0,zero,0x8002ee44
8002ee34:  _move v0,s1
8002ee38:  move s6,s0
8002ee3c:  move s3,v1
8002ee40:  move v0,s1
8002ee44:  lw s1,0x0(s1)
8002ee48:  nop
8002ee4c:  bne s1,zero,0x8002eda8
8002ee50:  _nop
8002ee54:  move v0,s5
8002ee58:  bne v0,zero,0x8002ee64
8002ee5c:  _nop
8002ee60:  move v0,s6
8002ee64:  lw ra,0x34(sp)
8002ee68:  lw s8,0x30(sp)
8002ee6c:  lw s7,0x2c(sp)
8002ee70:  lw s6,0x28(sp)
8002ee74:  lw s5,0x24(sp)
8002ee78:  lw s4,0x20(sp)
8002ee7c:  lw s3,0x1c(sp)
8002ee80:  lw s2,0x18(sp)
8002ee84:  lw s1,0x14(sp)
8002ee88:  lw s0,0x10(sp)
8002ee8c:  jr ra
8002ee90:  _addiu sp,sp,0x38
