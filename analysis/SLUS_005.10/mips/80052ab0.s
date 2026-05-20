# addr: 0x80052ab0  name: PRNT_OBJ_46C
80052ab0:  lw a3,0x40(sp)
80052ab4:  beq s8,zero,0x80052ac0
80052ab8:  _addu s4,s6,a3
80052abc:  addiu s4,s4,0x1
80052ac0:  andi v0,s3,0x40
80052ac4:  beq v0,zero,0x80052ad0
80052ac8:  _nop
80052acc:  addiu s4,s4,0x2
80052ad0:  lw s5,0x3c(sp)
80052ad4:  nop
80052ad8:  slt v0,s5,s4
80052adc:  beq v0,zero,0x80052ae8
80052ae0:  _andi v0,s3,0x30
80052ae4:  move s5,s4
80052ae8:  bne v0,zero,0x80052b24
80052aec:  _move a0,s8
80052af0:  lw a3,0x44(sp)
80052af4:  nop
80052af8:  beq a3,zero,0x80052b24
80052afc:  _slt v0,s5,a3
80052b00:  beq v0,zero,0x80052b24
80052b04:  _move s0,s5
80052b08:  jal 0x80052da4
80052b0c:  _li a0,0x20
80052b10:  lw a3,0x44(sp)
80052b14:  addiu s0,s0,0x1
80052b18:  slt v0,s0,a3
80052b1c:  bne v0,zero,0x80052b08
80052b20:  _move a0,s8
80052b24:  beq a0,zero,0x80052b38
80052b28:  _andi v0,s3,0x40
80052b2c:  jal 0x80052da4
80052b30:  _nop
80052b34:  andi v0,s3,0x40
80052b38:  beq v0,zero,0x80052b58
80052b3c:  _andi v1,s3,0x30
80052b40:  jal 0x80052da4
80052b44:  _li a0,0x30
80052b48:  lb a0,0x0(s7)
80052b4c:  jal 0x80052da4
80052b50:  _nop
80052b54:  andi v1,s3,0x30
80052b58:  li v0,0x20
80052b5c:  bne v1,v0,0x80052b94
80052b60:  _nop
80052b64:  lw a3,0x44(sp)
80052b68:  nop
80052b6c:  slt v0,s5,a3
80052b70:  beq v0,zero,0x80052b94
80052b74:  _move s0,s5
80052b78:  jal 0x80052da4
80052b7c:  _li a0,0x30
80052b80:  lw a3,0x44(sp)
80052b84:  addiu s0,s0,0x1
80052b88:  slt v0,s0,a3
80052b8c:  bne v0,zero,0x80052b78
80052b90:  _nop
80052b94:  lw a3,0x3c(sp)
80052b98:  move s0,s4
80052b9c:  slt v0,s0,a3
80052ba0:  beq v0,zero,0x80052bc4
80052ba4:  _nop
80052ba8:  jal 0x80052da4
80052bac:  _li a0,0x30
80052bb0:  lw a3,0x3c(sp)
80052bb4:  addiu s0,s0,0x1
80052bb8:  slt v0,s0,a3
80052bbc:  bne v0,zero,0x80052ba8
80052bc0:  _nop
80052bc4:  addiu s0,s6,-0x1
80052bc8:  bltz s0,0x80052bf8
80052bcc:  _nop
