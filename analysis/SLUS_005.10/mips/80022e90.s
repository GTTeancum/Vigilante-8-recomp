# addr: 0x80022e90  name: FUN_80022e90
80022e90:  lb v0,0x15(gp)
80022e94:  addiu sp,sp,-0x38
80022e98:  sw s2,0x28(sp)
80022e9c:  move s2,a0
80022ea0:  addiu v1,s2,0xa4
80022ea4:  sw ra,0x30(sp)
80022ea8:  sw s3,0x2c(sp)
80022eac:  sw s1,0x24(sp)
80022eb0:  bne v0,zero,0x8002303c
80022eb4:  _sw s0,0x20(sp)
80022eb8:  lbu v0,0xd0(s2)
80022ebc:  nop
80022ec0:  sltiu v0,v0,0x6
80022ec4:  bne v0,zero,0x80023040
80022ec8:  _clear v0
80022ecc:  lbu v0,0xf(v1)
80022ed0:  nop
80022ed4:  sll v0,v0,0x2
80022ed8:  addu v0,s2,v0
80022edc:  lw s3,0x110(v0)
80022ee0:  nop
80022ee4:  beq s3,zero,0x8002303c
80022ee8:  _li v0,0x4
80022eec:  lb v1,0x8(s3)
80022ef0:  nop
80022ef4:  bne v1,v0,0x80023040
80022ef8:  _clear v0
80022efc:  lw v0,0x24(s2)
80022f00:  lui v1,0xfff0
80022f04:  ori v1,v1,0x6000
80022f08:  addu v0,v0,v1
80022f0c:  sw v0,0x10(sp)
80022f10:  lw v0,0x24(s2)
80022f14:  lw a0,0x6fc(gp)
80022f18:  lui v1,0xf
80022f1c:  ori v1,v1,0xa000
80022f20:  addu v0,v0,v1
80022f24:  sw v0,0x14(sp)
80022f28:  lw v0,0x2c(s2)
80022f2c:  lh a1,0x6f0(gp)
80022f30:  subu v0,v0,v1
80022f34:  sw v0,0x18(sp)
80022f38:  lw v0,0x2c(s2)
80022f3c:  lh a2,0x7dc(gp)
80022f40:  addiu a3,sp,0x10
80022f44:  addu v0,v0,v1
80022f48:  jal 0x80021a30
80022f4c:  _sw v0,0x1c(sp)
80022f50:  move a0,v0
80022f54:  beq a0,zero,0x8002303c
80022f58:  _lui a1,0xf
80022f5c:  lw s1,0xe4(s2)
80022f60:  lw v0,0x24(a0)
80022f64:  lw v1,0x24(s1)
80022f68:  ori a1,a1,0x9fff
80022f6c:  subu v0,v0,v1
80022f70:  bgez v0,0x80022f7c
80022f74:  _nop
80022f78:  subu v0,zero,v0
80022f7c:  slt v0,a1,v0
80022f80:  bne v0,zero,0x80023040
80022f84:  _clear v0
80022f88:  lw v0,0x28(a0)
80022f8c:  lw v1,0x28(s1)
80022f90:  nop
80022f94:  subu v0,v0,v1
80022f98:  bgez v0,0x80022fa4
80022f9c:  _nop
80022fa0:  subu v0,zero,v0
80022fa4:  slt v0,a1,v0
80022fa8:  bne v0,zero,0x80023040
80022fac:  _clear v0
80022fb0:  lw v0,0x2c(a0)
80022fb4:  lw v1,0x2c(s1)
80022fb8:  nop
80022fbc:  subu v0,v0,v1
80022fc0:  bgez v0,0x80022fcc
80022fc4:  _nop
80022fc8:  subu v0,zero,v0
80022fcc:  slt v0,a1,v0
80022fd0:  bne v0,zero,0x80023040
80022fd4:  _clear v0
80022fd8:  sw a0,0xe4(s2)
80022fdc:  lw v0,0x64(s3)
80022fe0:  nop
80022fe4:  beq v0,zero,0x80023000
80022fe8:  _move a0,s3
80022fec:  li a1,0xb
80022ff0:  jalr v0
80022ff4:  _move a2,s2
80022ff8:  j 0x80023004
80022ffc:  _move s0,v0
80023000:  clear s0
80023004:  jal 0x80017160
80023008:  _nop
8002300c:  lb a0,0x16(gp)
80023010:  sll v0,v0,0x7
80023014:  li v1,0x2
80023018:  subu v1,v1,a0
8002301c:  mult v0,v1
80023020:  li v0,0x1
80023024:  mflo t0
80023028:  sra v1,t0,0xf
8002302c:  addu v1,s0,v1
80023030:  sh v1,0x6(s3)
80023034:  j 0x80023040
80023038:  _sw s1,0xe4(s2)
8002303c:  clear v0
80023040:  lw ra,0x30(sp)
80023044:  lw s3,0x2c(sp)
80023048:  lw s2,0x28(sp)
8002304c:  lw s1,0x24(sp)
80023050:  lw s0,0x20(sp)
80023054:  jr ra
80023058:  _addiu sp,sp,0x38
