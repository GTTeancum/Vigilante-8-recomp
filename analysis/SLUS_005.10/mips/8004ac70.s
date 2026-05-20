# addr: 0x8004ac70  name: BIOS_OBJ_16BC
8004ac70:  jal 0x800495b4
8004ac74:  _nop
8004ac78:  move s0,v0
8004ac7c:  beq s0,zero,0x8004ace8
8004ac80:  _andi v0,s0,0x4
8004ac84:  beq v0,zero,0x8004acb4
8004ac88:  _andi v0,s0,0x2
8004ac8c:  lui v0,0x8006
8004ac90:  lw v0,0x80(v0)
8004ac94:  nop
8004ac98:  beq v0,zero,0x8004acb0
8004ac9c:  _nop
8004aca0:  lbu a0,0x0(s1)
8004aca4:  lui a1,0x800a
8004aca8:  jalr v0
8004acac:  _addiu a1,a1,0x3248
8004acb0:  andi v0,s0,0x2
8004acb4:  beq v0,zero,0x8004ac70
8004acb8:  _nop
8004acbc:  lui v0,0x8006
8004acc0:  lw v0,0x7c(v0)
8004acc4:  nop
8004acc8:  beq v0,zero,0x8004ac70
8004accc:  _nop
8004acd0:  lbu a0,0x0(s3)
8004acd4:  lui a1,0x800a
8004acd8:  jalr v0
8004acdc:  _addiu a1,a1,0x3240
8004ace0:  j 0x8004ac70
8004ace4:  _nop
8004ace8:  lui v0,0x8006
8004acec:  lw v0,0x344(v0)
8004acf0:  nop
8004acf4:  sb s2,0x0(v0)
8004acf8:  lw ra,0x20(sp)
8004acfc:  lw s3,0x1c(sp)
8004ad00:  lw s2,0x18(sp)
8004ad04:  lw s1,0x14(sp)
8004ad08:  lw s0,0x10(sp)
8004ad0c:  jr ra
8004ad10:  _addiu sp,sp,0x28
