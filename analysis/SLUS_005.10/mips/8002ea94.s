# addr: 0x8002ea94  name: FUN_8002ea94
8002ea94:  lw v0,0x10(gp)
8002ea98:  addiu sp,sp,-0x50
8002ea9c:  sw s3,0x34(sp)
8002eaa0:  move s3,a0
8002eaa4:  addiu a3,s3,0xa4
8002eaa8:  sw ra,0x4c(sp)
8002eaac:  sw s8,0x48(sp)
8002eab0:  sw s7,0x44(sp)
8002eab4:  sw s6,0x40(sp)
8002eab8:  sw s5,0x3c(sp)
8002eabc:  sw s4,0x38(sp)
8002eac0:  sw s2,0x30(sp)
8002eac4:  sw s1,0x2c(sp)
8002eac8:  sw s0,0x28(sp)
8002eacc:  sw a1,0x54(sp)
8002ead0:  beq v0,zero,0x8002eaf0
8002ead4:  _sw a3,0x20(sp)
8002ead8:  lh v1,0x6(s3)
8002eadc:  li v0,-0x1
8002eae0:  bne v1,v0,0x8002eaf0
8002eae4:  _lui v0,0x8007
8002eae8:  j 0x8002eaf8
8002eaec:  _addiu v0,v0,-0x960
8002eaf0:  lui v0,0x8007
8002eaf4:  addiu v0,v0,-0x980
8002eaf8:  clear s5
8002eafc:  move s2,s5
8002eb00:  li s4,-0x1
8002eb04:  SetRotMatrix v0
8002eb2c:  SetTransMatrix v0
8002eb44:  lw v0,0x714(gp)
8002eb48:  nop
8002eb4c:  lw s1,0x0(v0)
8002eb50:  nop
8002eb54:  beq s1,zero,0x8002ec88
8002eb58:  _nop
8002eb5c:  li s8,0x3
8002eb60:  lw s0,0x8(v0)
8002eb64:  nop
8002eb68:  beq s0,s3,0x8002ec78
8002eb6c:  _move v0,s1
8002eb70:  lbu v0,0x4(s0)
8002eb74:  nop
8002eb78:  beq v0,s8,0x8002ec78
8002eb7c:  _move v0,s1
8002eb80:  lw v0,0x0(s0)
8002eb84:  nop
8002eb88:  andi v0,v0,0x4000
8002eb8c:  beq v0,zero,0x8002ec78
8002eb90:  _move v0,s1
8002eb94:  lh v0,0x6(s0)
8002eb98:  nop
8002eb9c:  bgtz v0,0x8002ebb8
8002eba0:  _addiu a0,s0,0x48
8002eba4:  lb v0,0x15(gp)
8002eba8:  nop
8002ebac:  bne v0,s8,0x8002ec78
8002ebb0:  _move v0,s1
8002ebb4:  addiu a0,s0,0x48
8002ebb8:  jal 0x800432d0
8002ebbc:  _addiu a1,sp,0x10
8002ebc0:  lw v0,0x14(sp)
8002ebc4:  lw v1,0x10(sp)
8002ebc8:  sra v0,v0,0xa
8002ebcc:  bgez v0,0x8002ebd8
8002ebd0:  _move a0,v0
8002ebd4:  subu a0,zero,a0
8002ebd8:  sra v1,v1,0xa
8002ebdc:  bgez v1,0x8002ebe8
8002ebe0:  _nop
8002ebe4:  subu v1,zero,v1
8002ebe8:  slt v0,a0,v1
8002ebec:  beq v0,zero,0x8002ebf8
8002ebf0:  _nop
8002ebf4:  move a0,v1
8002ebf8:  lw v0,0x18(sp)
8002ebfc:  nop
8002ec00:  sra v1,v0,0xa
8002ec04:  slt v0,a0,v1
8002ec08:  beq v0,zero,0x8002ec48
8002ec0c:  _nop
8002ec10:  beq s2,zero,0x8002ec38
8002ec14:  _mult v1,s7
8002ec18:  mflo v0
8002ec1c:  nop
8002ec20:  nop
8002ec24:  mult s6,a0
8002ec28:  mflo t0
8002ec2c:  slt v0,t0,v0
8002ec30:  beq v0,zero,0x8002ec48
8002ec34:  _nop
8002ec38:  move s2,s0
8002ec3c:  move s7,a0
8002ec40:  j 0x8002ec74
8002ec44:  _move s6,v1
8002ec48:  bne s2,zero,0x8002ec78
8002ec4c:  _move v0,s1
8002ec50:  addiu a0,s3,0x24
8002ec54:  jal 0x80016aac
8002ec58:  _addiu a1,s0,0x48
8002ec5c:  move v1,v0
8002ec60:  sltu v0,v1,s4
8002ec64:  beq v0,zero,0x8002ec78
8002ec68:  _move v0,s1
8002ec6c:  move s4,v1
8002ec70:  move s5,s0
8002ec74:  move v0,s1
8002ec78:  lw s1,0x0(s1)
8002ec7c:  nop
8002ec80:  bne s1,zero,0x8002eb60
8002ec84:  _nop
8002ec88:  bne s2,zero,0x8002ec94
8002ec8c:  _nop
8002ec90:  move s2,s5
8002ec94:  lw v0,0xe4(s3)
8002ec98:  nop
8002ec9c:  beq s2,v0,0x8002ed04
8002eca0:  _nop
8002eca4:  bne s2,zero,0x8002ecbc
8002eca8:  _nop
8002ecac:  lw a3,0x54(sp)
8002ecb0:  nop
8002ecb4:  beq a3,zero,0x8002ed04
8002ecb8:  _nop
8002ecbc:  sw s2,0xe4(s3)
8002ecc0:  lw a3,0x20(sp)
8002ecc4:  nop
8002ecc8:  lbu v0,0xf(a3)
8002eccc:  sh zero,0x18(a3)
8002ecd0:  addiu v0,v0,0x9
8002ecd4:  sll v0,v0,0x2
8002ecd8:  addu v0,s3,v0
8002ecdc:  lw a0,0xec(v0)
8002ece0:  nop
8002ece4:  beq a0,zero,0x8002ed04
8002ece8:  _nop
8002ecec:  lw v0,0x64(a0)
8002ecf0:  nop
8002ecf4:  beq v0,zero,0x8002ed04
8002ecf8:  _li a1,0xa
8002ecfc:  jalr v0
8002ed00:  _clear a2
8002ed04:  lw ra,0x4c(sp)
8002ed08:  lw s8,0x48(sp)
8002ed0c:  lw s7,0x44(sp)
8002ed10:  lw s6,0x40(sp)
8002ed14:  lw s5,0x3c(sp)
8002ed18:  lw s4,0x38(sp)
8002ed1c:  lw s3,0x34(sp)
8002ed20:  lw s2,0x30(sp)
8002ed24:  lw s1,0x2c(sp)
8002ed28:  lw s0,0x28(sp)
8002ed2c:  jr ra
8002ed30:  _addiu sp,sp,0x50
