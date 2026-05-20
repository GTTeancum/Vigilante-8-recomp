# addr: 0x80041da0  name: FUN_80041da0
80041da0:  addiu sp,sp,-0x28
80041da4:  sw s2,0x18(sp)
80041da8:  move s2,a0
80041dac:  sw ra,0x20(sp)
80041db0:  sw s3,0x1c(sp)
80041db4:  sw s1,0x14(sp)
80041db8:  sw s0,0x10(sp)
80041dbc:  lh v0,0x12(s2)
80041dc0:  nop
80041dc4:  blez v0,0x80041e34
80041dc8:  _clear s0
80041dcc:  li s3,0x1
80041dd0:  move s1,s2
80041dd4:  lw a0,0x1c(s1)
80041dd8:  nop
80041ddc:  lw v0,0x4(a0)
80041de0:  nop
80041de4:  xor v0,v0,s2
80041de8:  sltiu v0,v0,0x1
80041dec:  subu v1,s3,v0
80041df0:  sll v1,v1,0x2
80041df4:  addu v1,a0,v1
80041df8:  lw v1,0x0(v1)
80041dfc:  nop
80041e00:  bne v1,zero,0x80041e18
80041e04:  _sll v0,v0,0x2
80041e08:  jal 0x80045088
80041e0c:  _nop
80041e10:  j 0x80041e20
80041e14:  _nop
80041e18:  addu v0,a0,v0
80041e1c:  sw zero,0x0(v0)
80041e20:  lh v0,0x12(s2)
80041e24:  addiu s0,s0,0x1
80041e28:  slt v0,s0,v0
80041e2c:  bne v0,zero,0x80041dd4
80041e30:  _addiu s1,s1,0x4
80041e34:  lw v0,0x18(s2)
80041e38:  nop
80041e3c:  beq v0,zero,0x80041e5c
80041e40:  _nop
80041e44:  lw a0,0x8(v0)
80041e48:  jal 0x80045088
80041e4c:  _nop
80041e50:  lw a0,0x18(s2)
80041e54:  jal 0x8001bddc
80041e58:  _nop
80041e5c:  jal 0x80045088
80041e60:  _move a0,s2
80041e64:  lw ra,0x20(sp)
80041e68:  lw s3,0x1c(sp)
80041e6c:  lw s2,0x18(sp)
80041e70:  lw s1,0x14(sp)
80041e74:  lw s0,0x10(sp)
80041e78:  jr ra
80041e7c:  _addiu sp,sp,0x28
