# addr: 0x80052bd0  name: PRNT_OBJ_58C
80052bd0:  lb a0,0x0(s2)
80052bd4:  addiu s2,s2,0x1
80052bd8:  jal 0x80052da4
80052bdc:  _addiu s0,s0,-0x1
80052be0:  bltz s0,0x80052bf8
80052be4:  _nop
80052be8:  j 0x80052bd0
80052bec:  _nop
80052bf0:  jal 0x80052da4
80052bf4:  _li a0,0x30
80052bf8:  lw a3,0x40(sp)
80052bfc:  nop
80052c00:  addiu a3,a3,-0x1
80052c04:  bgez a3,0x80052bf0
80052c08:  _sw a3,0x40(sp)
80052c0c:  andi v0,s3,0x10
80052c10:  beq v0,zero,0x80052c48
80052c14:  _nop
80052c18:  lw a3,0x44(sp)
80052c1c:  nop
80052c20:  slt v0,s5,a3
80052c24:  beq v0,zero,0x80052c48
80052c28:  _move s0,s5
80052c2c:  jal 0x80052da4
80052c30:  _li a0,0x20
80052c34:  lw a3,0x44(sp)
80052c38:  addiu s0,s0,0x1
80052c3c:  slt v0,s0,a3
80052c40:  bne v0,zero,0x80052c2c
80052c44:  _nop
80052c48:  lw v1,0x44(sp)
80052c4c:  nop
80052c50:  slt v0,v1,s5
80052c54:  beq v0,zero,0x80052c60
80052c58:  _nop
80052c5c:  move v1,s5
80052c60:  lw a3,0x38(sp)
80052c64:  nop
80052c68:  addu a3,a3,v1
80052c6c:  j 0x80052ca0
80052c70:  _sw a3,0x38(sp)
