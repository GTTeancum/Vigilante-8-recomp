# addr: 0x80100d1c  name: FUN_80100d1c
80100d1c:  addiu sp,sp,-0x28
80100d20:  sw s1,0x14(sp)
80100d24:  move s1,a0
80100d28:  sw s0,0x10(sp)
80100d2c:  move s0,a2
80100d30:  sw s3,0x1c(sp)
80100d34:  addiu s3,sp,0x2c
80100d38:  move a0,s3
80100d3c:  sw ra,0x20(sp)
80100d40:  sw s2,0x18(sp)
80100d44:  jal 0x800224ec
80100d48:  _sw a1,0x2c(sp)
80100d4c:  slti s0,s0,0x5
80100d50:  bne s0,zero,0x80100d68
80100d54:  _move s2,v0
80100d58:  jal 0x800224ec
80100d5c:  _move a0,s3
80100d60:  slti a0,zero,0x35b
80100d64:  move a0,v0
80100d68:  move a0,s2
80100d6c:  lbu v1,0x4(s1)
80100d70:  li v0,0x5
80100d74:  bne v1,v0,0x80100dc8
80100d78:  _nop
80100d7c:  lh v0,0x6(s1)
80100d80:  nop
80100d84:  bltz v0,0x80100dc8
80100d88:  _nop
80100d8c:  lui v0,0x8006
80100d90:  lb v0,0x531a(v0)
80100d94:  nop
80100d98:  addiu v1,v0,0x2
80100d9c:  mult s2,v1
80100da0:  mflo v0
80100da4:  nop
80100da8:  bgez v0,0x80100db4
80100dac:  _mult a0,v1
80100db0:  addiu v0,v0,0x3
80100db4:  mflo v1
80100db8:  bgez v1,0x80100dc4
80100dbc:  _sra s2,v0,0x2
80100dc0:  addiu v1,v1,0x3
80100dc4:  sra a0,v1,0x2
80100dc8:  sh s2,0xc(s1)
80100dcc:  sh a0,0xe(s1)
80100dd0:  lw s1,0x38(s1)
80100dd4:  nop
80100dd8:  beq s1,zero,0x80100e04
80100ddc:  _nop
80100de0:  lhu v0,0xc(s1)
80100de4:  nop
80100de8:  bne v0,zero,0x80100df4
80100dec:  _nop
80100df0:  sh s2,0xc(s1)
80100df4:  lw s1,0x34(s1)
80100df8:  nop
80100dfc:  bne s1,zero,0x80100de0
80100e00:  _nop
80100e04:  lw ra,0x20(sp)
80100e08:  lw s3,0x1c(sp)
80100e0c:  lw s2,0x18(sp)
80100e10:  lw s1,0x14(sp)
80100e14:  lw s0,0x10(sp)
80100e18:  jr ra
80100e1c:  _addiu sp,sp,0x28
