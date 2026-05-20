# addr: 0x80041e80  name: FUN_80041e80
80041e80:  lw v0,0x8cc(gp)
80041e84:  addiu sp,sp,-0x20
80041e88:  lui v1,0x8006
80041e8c:  addiu v1,v1,0x5bc8
80041e90:  sw ra,0x18(sp)
80041e94:  sw s1,0x14(sp)
80041e98:  beq v0,v1,0x80041ed4
80041e9c:  _sw s0,0x10(sp)
80041ea0:  move s0,v1
80041ea4:  lw a0,0x8c4(gp)
80041ea8:  nop
80041eac:  lw v1,0x4(a0)
80041eb0:  lw v0,0x0(a0)
80041eb4:  nop
80041eb8:  sw v1,0x4(v0)
80041ebc:  jal 0x80040e18
80041ec0:  _sw v0,0x0(v1)
80041ec4:  lw v0,0x8cc(gp)
80041ec8:  nop
80041ecc:  bne v0,s0,0x80041ea4
80041ed0:  _nop
80041ed4:  lw v0,0x8c0(gp)
80041ed8:  nop
80041edc:  blez v0,0x80041f10
80041ee0:  _clear s0
80041ee4:  lw v0,0x8d4(gp)
80041ee8:  sll v1,s0,0x2
80041eec:  addu v1,v1,v0
80041ef0:  lw a0,0x0(v1)
80041ef4:  jal 0x80041da0
80041ef8:  _addiu s0,s0,0x1
80041efc:  lw v0,0x8c0(gp)
80041f00:  nop
80041f04:  slt v0,s0,v0
80041f08:  bne v0,zero,0x80041ee4
80041f0c:  _nop
80041f10:  lw a0,0x8d4(gp)
80041f14:  nop
80041f18:  beq a0,zero,0x80041f28
80041f1c:  _nop
80041f20:  jal 0x80045088
80041f24:  _nop
80041f28:  lw v0,0x8bc(gp)
80041f2c:  nop
80041f30:  blez v0,0x80041fa8
80041f34:  _clear s0
80041f38:  move s1,s0
80041f3c:  lw v0,0x8d0(gp)
80041f40:  nop
80041f44:  addu a0,s1,v0
80041f48:  lhu v0,0x0(a0)
80041f4c:  nop
80041f50:  beq v0,zero,0x80041f94
80041f54:  _nop
80041f58:  lw a0,0xc(a0)
80041f5c:  nop
80041f60:  beq a0,zero,0x80041f70
80041f64:  _nop
80041f68:  jal 0x80045088
80041f6c:  _nop
80041f70:  lw v0,0x8d0(gp)
80041f74:  nop
80041f78:  addu v0,s1,v0
80041f7c:  lw a0,0x10(v0)
80041f80:  nop
80041f84:  beq a0,zero,0x80041f94
80041f88:  _nop
80041f8c:  jal 0x80045088
80041f90:  _nop
80041f94:  lw v0,0x8bc(gp)
80041f98:  addiu s0,s0,0x1
80041f9c:  slt v0,s0,v0
80041fa0:  bne v0,zero,0x80041f3c
80041fa4:  _addiu s1,s1,0x34
80041fa8:  lw a0,0x8d0(gp)
80041fac:  nop
80041fb0:  beq a0,zero,0x80041fc0
80041fb4:  _nop
80041fb8:  jal 0x80045088
80041fbc:  _nop
80041fc0:  lw ra,0x18(sp)
80041fc4:  lw s1,0x14(sp)
80041fc8:  lw s0,0x10(sp)
80041fcc:  jr ra
80041fd0:  _addiu sp,sp,0x20
